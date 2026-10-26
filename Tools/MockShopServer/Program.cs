// SoulReaper 상점 클라이언트(IShopApi Http 구현)를 시험하기 위한 로컬 목(Mock) 서버.
// 실행: dotnet run   (기본 주소 http://localhost:5080, 바꾸려면 dotnet run -- --urls http://localhost:6000)
//
// 설계 메모
// - 계정(지갑) 상태는 메모리에만 있다. 서버를 재시작하거나 POST /admin/reset 을 부르면 초기화된다.
// - 가격은 클라이언트가 보낸 price 를 그대로 믿는다. (카탈로그는 클라이언트 DT 에 있고, 이 서버는 거래 확정만 흉내 낸다.)
// - 모든 거래 응답에 최신 지갑 스냅샷(gold, inventory)을 싣는다. 실패 응답에도 싣는다.
// - 의도적 실패는 쿼리로 지정한다:  ?delay=3000   ?fail=timeout|drop|500|503|badjson|rejected

using System.Text.Json.Serialization;

var builder = WebApplication.CreateBuilder(args);
builder.WebHost.UseUrls(builder.Configuration["urls"] ?? "http://localhost:5080");
builder.Services.ConfigureHttpJsonOptions(o =>
    o.SerializerOptions.DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull);

var app = builder.Build();

int startingGold = app.Configuration.GetValue("StartingGold", 1000);   // 클라이언트 UShopSettings::StartingGold 와 맞출 것
int maxBagKinds = app.Configuration.GetValue("MaxBagKinds", 3);        // 클라이언트 UShopSettings::MaxBagKinds 와 맞출 것

// 클라이언트는 요청 URL 에 ?fail= 을 붙이지 않으므로, 서버를 켤 때 실패 모드를 정할 수 있게 한다.
//   dotnet run -- --DefaultFail=500                         모든 요청이 500
//   dotnet run -- --DefaultFail=timeout --FailPath=/wallet  /wallet 만 응답 없음
//   dotnet run -- --DefaultDelay=3000                       모든 요청을 3초 지연
// 요청 쿼리(?fail=, ?delay=)가 있으면 그쪽이 우선한다.
string defaultFail = (app.Configuration["DefaultFail"] ?? "").ToLowerInvariant();
int defaultDelayMs = app.Configuration.GetValue("DefaultDelay", 0);
string failPath = app.Configuration["FailPath"] ?? "";
//   dotnet run -- --DefaultFail=500 --FailPath=/checkout --FailAccount=_pie1   계정 ID 에 _pie1 이 들어간 요청만 실패 (PIE 에서 한 명만 실패시킬 때)
string failAccount = app.Configuration["FailAccount"] ?? "";

var accounts = new Dictionary<string, Account>();
var gate = new object();

Account GetOrCreate(string accountId)
{
    if (!accounts.TryGetValue(accountId, out var account))
    {
        account = new Account { Gold = startingGold };
        accounts[accountId] = account;
    }
    return account;
}

Snapshot MakeSnapshot(bool ok, string? error, Account account) =>
    new(ok, error, account.Gold, new Dictionary<string, int>(account.Inventory));

// delay / fail 쿼리를 먼저 처리하고, 정상이면 logic 을 실행한다.
async Task<IResult> Run(HttpContext ctx, string label, Func<IResult> logic, string? accountId = null)
{
    string fail = ctx.Request.Query["fail"].ToString().ToLowerInvariant();
    int.TryParse(ctx.Request.Query["delay"], out int delayMs);

    bool useDefaults = failPath.Length == 0 || string.Equals(ctx.Request.Path.Value, failPath, StringComparison.OrdinalIgnoreCase);
    if (failAccount.Length > 0 && (accountId ?? "").Contains(failAccount, StringComparison.OrdinalIgnoreCase) == false)
        useDefaults = false;
    if (useDefaults)
    {
        if (fail.Length == 0)
            fail = defaultFail;
        if (delayMs == 0)
            delayMs = defaultDelayMs;
    }

    Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] {label}  delay={delayMs}  fail={(fail.Length == 0 ? "none" : fail)}");

    try
    {
        if (delayMs > 0)
            await Task.Delay(delayMs, ctx.RequestAborted);

        switch (fail)
        {
            case "timeout":
                // 응답을 영영 보내지 않는다. 클라이언트가 타임아웃으로 연결을 닫으면 풀린다.
                await Task.Delay(Timeout.Infinite, ctx.RequestAborted);
                return Results.Empty;
            case "drop":
                ctx.Abort();   // 연결을 갑자기 끊는다
                return Results.Empty;
            case "500":
                return Results.Json(new { ok = false, error = "INTERNAL_ERROR" }, statusCode: 500);
            case "503":
                return Results.Json(new { ok = false, error = "SERVICE_UNAVAILABLE" }, statusCode: 503);
            case "badjson":
                return Results.Text("{ this is not valid json", "application/json", statusCode: 200);
            case "rejected":
                return Results.Json(new { ok = false, error = "REJECTED" }, statusCode: 409);
        }

        return logic();
    }
    catch (OperationCanceledException)
    {
        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] {label}  -> client closed the connection");
        return Results.Empty;
    }
}

// HttpContext 하나만 받는 람다는 RequestDelegate 로 해석되어 반환값(IResult)이 버려진다.
// Func<> 변수로 한 번 받아서 반환값이 응답으로 쓰이게 한다.
Func<HttpContext, Task<IResult>> health = ctx =>
    Run(ctx, "GET /health", () => Results.Json(new { ok = true }));
app.MapGet("/health", health);

app.MapGet("/wallet", (HttpContext ctx, string? accountId) =>
    Run(ctx, $"GET /wallet account={accountId}", () =>
    {
        if (string.IsNullOrWhiteSpace(accountId))
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            return Results.Json(MakeSnapshot(true, null, GetOrCreate(accountId)));
        }
    }, accountId));

app.MapPost("/buy", (HttpContext ctx, TradeRequest req) =>
    Run(ctx, $"POST /buy account={req.AccountId} row={req.RowName} price={req.Price}", () =>
    {
        if (string.IsNullOrWhiteSpace(req.AccountId) || string.IsNullOrWhiteSpace(req.RowName) || req.Price < 0)
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            var account = GetOrCreate(req.AccountId);

            if (!account.Inventory.ContainsKey(req.RowName) && account.Inventory.Count >= maxBagKinds)
                return Results.Json(MakeSnapshot(false, "BAG_FULL", account), statusCode: 409);

            if (account.Gold < req.Price)
                return Results.Json(MakeSnapshot(false, "NOT_ENOUGH_GOLD", account), statusCode: 409);

            account.Gold -= req.Price;
            account.Inventory[req.RowName] = account.Inventory.GetValueOrDefault(req.RowName) + 1;
            return Results.Json(MakeSnapshot(true, null, account));
        }
    }, req.AccountId));

app.MapPost("/refund", (HttpContext ctx, TradeRequest req) =>
    Run(ctx, $"POST /refund account={req.AccountId} row={req.RowName} price={req.Price}", () =>
    {
        if (string.IsNullOrWhiteSpace(req.AccountId) || string.IsNullOrWhiteSpace(req.RowName) || req.Price < 0)
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            var account = GetOrCreate(req.AccountId);

            if (!account.Inventory.TryGetValue(req.RowName, out int count) || count <= 0)
                return Results.Json(MakeSnapshot(false, "NOT_OWNED", account), statusCode: 409);

            account.Gold += req.Price;
            if (count - 1 <= 0)
                account.Inventory.Remove(req.RowName);   // 0개짜리 키를 남기지 않는다
            else
                account.Inventory[req.RowName] = count - 1;

            return Results.Json(MakeSnapshot(true, null, account));
        }
    }, req.AccountId));

// 클라이언트가 보낸 지갑({gold, inventory})을 그대로 저장한다.
// 치트 검증은 이 서버의 역할이 아니다. (실서비스라면 BFunc 같은 별도 검증 코드가 담당)
// 같은 값을 여러 번 보내도 결과가 같다 (멱등).
app.MapPost("/checkout", (HttpContext ctx, SaveWalletRequest req) =>
    Run(ctx, $"POST /checkout account={req.AccountId} gold={req.Gold} kinds={req.Inventory?.Count ?? 0}", () =>
    {
        if (string.IsNullOrWhiteSpace(req.AccountId) || req.Gold is null || req.Gold < 0 ||
            req.Inventory is null || req.Inventory.Values.Any(c => c < 0))
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            var account = GetOrCreate(req.AccountId);
            account.Gold = req.Gold.Value;
            account.Inventory = req.Inventory.Where(p => p.Value > 0).ToDictionary(p => p.Key, p => p.Value);

            return Results.Json(MakeSnapshot(true, null, account));
        }
    }, req.AccountId));

Func<HttpContext, Task<IResult>> reset = ctx =>
    Run(ctx, "POST /admin/reset", () =>
    {
        lock (gate)
        {
            accounts.Clear();
        }
        return Results.Json(new { ok = true });
    });
app.MapPost("/admin/reset", reset);

Console.WriteLine($"MockShopServer  startingGold={startingGold}  maxBagKinds={maxBagKinds}");
app.Run();

record TradeRequest(string? AccountId, string? RowName, int Price);
record SaveWalletRequest(string? AccountId, int? Gold, Dictionary<string, int>? Inventory);
record Snapshot(bool Ok, string? Error, int Gold, Dictionary<string, int> Inventory);

class Account
{
    public int Gold;
    public Dictionary<string, int> Inventory = new();
}
