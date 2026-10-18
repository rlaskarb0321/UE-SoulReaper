# MockShopServer

SoulReaper 상점 클라이언트(`IShopApi` Http 구현)를 시험하기 위한 로컬 목 서버. 상태는 메모리에만 있다.

```
dotnet run                                # http://localhost:5080
dotnet run -- --urls http://localhost:6000
dotnet run -- --StartingGold=500 --MaxBagKinds=2
```

`StartingGold`, `MaxBagKinds` 기본값(1000, 3)은 클라이언트 `UShopSettings`와 맞춰 둔 값이다.

## 엔드포인트

| 메서드 | 경로 | 본문 | 설명 |
|---|---|---|---|
| GET | `/health` | - | 생존 확인 |
| GET | `/wallet?playerId=` | - | 지갑 스냅샷 (계정이 없으면 생성) |
| POST | `/buy` | `{playerId, rowName, price}` | 구매 확정 |
| POST | `/refund` | `{playerId, rowName, price}` | 환불 확정 |
| POST | `/admin/reset` | - | 모든 계정 초기화 |

가격(`price`)은 클라이언트 값을 그대로 믿는다. 카탈로그는 클라이언트 DT에 있다.

## 응답

성공/거절 모두 최신 지갑 스냅샷을 싣는다.

```json
{ "ok": true,  "gold": 700, "inventory": { "Heal": 1 } }
{ "ok": false, "error": "NOT_ENOUGH_GOLD", "gold": 0, "inventory": { "Heal": 2 } }
```

| 상태 | error | 의미 |
|---|---|---|
| 200 | - | 성공 |
| 400 | `INVALID_REQUEST` | 필드 누락 / 음수 가격 (스냅샷 없음) |
| 409 | `NOT_ENOUGH_GOLD` / `BAG_FULL` / `NOT_OWNED` | 규칙 위반 |

## 의도적 실패 (쿼리)

`/buy?fail=timeout&delay=...` 처럼 모든 엔드포인트에 붙일 수 있다. `delay`는 먼저 적용된다.

| 쿼리 | 서버 동작 | 클라이언트가 보는 것 |
|---|---|---|
| `delay=3000` | 3초 뒤 정상 처리 | 느린 응답 |
| `fail=500` / `fail=503` | 해당 상태 코드 응답 | 연결 성공 + 5xx |
| `fail=rejected` | 409 `REJECTED` (스냅샷 없음) | 연결 성공 + 거절 |
| `fail=badjson` | 200 + 깨진 JSON | 연결/상태는 정상, 파싱 실패 |
| `fail=drop` | 연결을 갑자기 끊음 | 연결 오류 |
| `fail=timeout` | 응답을 보내지 않음 | 클라이언트 타임아웃(`SetTimeout`)으로 실패 |

실패 모드에서는 지갑 상태가 변하지 않는다. 타임아웃을 시험하려면 클라이언트 타임아웃이 서버 지연보다 짧아야 한다.
