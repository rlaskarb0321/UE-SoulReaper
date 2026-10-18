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

var accounts = new Dictionary<string, Account>();
var gate = new object();

Account GetOrCreate(string playerId)
{
    if (!accounts.TryGetValue(playerId, out var account))
    {
        account = new Account { Gold = startingGold };
        accounts[playerId] = account;
    }
    return account;
}

Snapshot MakeSnapshot(bool ok, string? error, Account account) =>
    new(ok, error, account.Gold, new Dictionary<string, int>(account.Inventory));

// delay / fail 쿼리를 먼저 처리하고, 정상이면 logic 을 실행한다.
async Task<IResult> Run(HttpContext ctx, string label, Func<IResult> logic)
{
    string fail = ctx.Request.Query["fail"].ToString().ToLowerInvariant();
    int.TryParse(ctx.Request.Query["delay"], out int delayMs);

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

app.MapGet("/wallet", (HttpContext ctx, string? playerId) =>
    Run(ctx, $"GET /wallet player={playerId}", () =>
    {
        if (string.IsNullOrWhiteSpace(playerId))
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            return Results.Json(MakeSnapshot(true, null, GetOrCreate(playerId)));
        }
    }));

app.MapPost("/buy", (HttpContext ctx, TradeRequest req) =>
    Run(ctx, $"POST /buy player={req.PlayerId} row={req.RowName} price={req.Price}", () =>
    {
        if (string.IsNullOrWhiteSpace(req.PlayerId) || string.IsNullOrWhiteSpace(req.RowName) || req.Price < 0)
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            var account = GetOrCreate(req.PlayerId);

            if (!account.Inventory.ContainsKey(req.RowName) && account.Inventory.Count >= maxBagKinds)
                return Results.Json(MakeSnapshot(false, "BAG_FULL", account), statusCode: 409);

            if (account.Gold < req.Price)
                return Results.Json(MakeSnapshot(false, "NOT_ENOUGH_GOLD", account), statusCode: 409);

            account.Gold -= req.Price;
            account.Inventory[req.RowName] = account.Inventory.GetValueOrDefault(req.RowName) + 1;
            return Results.Json(MakeSnapshot(true, null, account));
        }
    }));

app.MapPost("/refund", (HttpContext ctx, TradeRequest req) =>
    Run(ctx, $"POST /refund player={req.PlayerId} row={req.RowName} price={req.Price}", () =>
    {
        if (string.IsNullOrWhiteSpace(req.PlayerId) || string.IsNullOrWhiteSpace(req.RowName) || req.Price < 0)
            return Results.Json(new { ok = false, error = "INVALID_REQUEST" }, statusCode: 400);

        lock (gate)
        {
            var account = GetOrCreate(req.PlayerId);

            if (!account.Inventory.TryGetValue(req.RowName, out int count) || count <= 0)
                return Results.Json(MakeSnapshot(false, "NOT_OWNED", account), statusCode: 409);

            account.Gold += req.Price;
            if (count - 1 <= 0)
                account.Inventory.Remove(req.RowName);   // 0개짜리 키를 남기지 않는다
            else
                account.Inventory[req.RowName] = count - 1;

            return Results.Json(MakeSnapshot(true, null, account));
        }
    }));

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

record TradeRequest(string? PlayerId, string? RowName, int Price);
record Snapshot(bool Ok, string? Error, int Gold, Dictionary<string, int> Inventory);

class Account
{
    public int Gold;
    public Dictionary<string, int> Inventory = new();
}
