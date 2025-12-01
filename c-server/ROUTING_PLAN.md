# 경로별 응답 추가 계획 (net 라우터 방식)

## 목표
- `net` 레이어가 요청 라인 파싱과 라우팅 테이블(add_route 스타일)을 관리해 `/`, `/health`, `/ping`, 404 등을 구분해 응답한다.
- 라우트 정의마다 상태 코드/콘텐츠 타입/본문을 들고 있고, 매 요청마다 `create_response(status, content_type, body)`로 응답을 빌드한다(버퍼는 `HttpContext`가 재사용).

## 구현 단계
1) **요청 파싱 (`net`)**  
   - 기존 `discard_request`를 대체할 `read_request(int fd, NetRequest *out)` 추가.  
   - `METHOD SP PATH SP HTTP/... CRLF`에서 메서드·경로를 추출하고 헤더/바디는 버퍼로 드레이닝.  
   - 파싱 실패 시 에러 코드 반환 → 호출자가 400/close 처리.

2) **라우터 추가 (`net`)**  
   - `NetRouter`에 `add_route(NetRouter*, const char *path, int (*handler)(HttpContext *http, const NetRequest *req))`와 `match(NetRouter*, const char *path)` 제공.  
   - 라우터 생성은 `NetContext`의 메서드 형태(`net.create_router()`)로 제공.  
   - 핸들러는 즉시 `http.create_response(http, status, content_type, body)`를 호출해 `HttpContext` 내부 버퍼를 채우고, 성공/실패를 반환.  
   - 사용 예시:  
     ```c
     NetRouter router = net.create_router();
     int handle_root(HttpContext *http, const NetRequest *req) {
         return http->create_response(http, 200, "text/html", DEFAULT_BODY);
     }
     int handle_health(HttpContext *http, const NetRequest *req) {
         return http->create_response(http, 200, "text/plain", "ok\n");
     }
     int handle_ping(HttpContext *http, const NetRequest *req) {
         return http->create_response(http, 200, "text/plain", "pong\n");
     }
     router.add_route(&router, "/", handle_root);
     router.add_route(&router, "/health", handle_health);
     router.add_route(&router, "/ping", handle_ping);
     ```

3) **HTTP 응답 확장 (`http`)**  
   - `create_response` 시그니처를 `int create_response(HttpContext*, int status, const char *content_type, const char *body);`로 확장.  
   - 호출 시 기존 버퍼를 파기 후 새 헤더/본문으로 재구성하고, 결과는 `ctx->response`에 저장.  
   - 라우트 핸들러 및 400/404 처리에서 동일 API를 사용.

4) **서버 루프 연결 (`server.c`)**  
   - 서버 초기화 시 `net.create_router()`로 라우터를 만들고 `add_route`로 핸들러 등록.  
   - 루프: `read_request`로 `NetRequest` 채움 → 메서드 검증(GET만) → `router.match`로 핸들러 선택 → 없으면 404 핸들러 → 파싱 실패면 400 핸들러.  
   - 핸들러가 `http.create_response`를 통해 `ctx->response`를 채우면 `net.send_all`로 전송.  
   - 정리 플로우(`goto cleanup`, `net.close`, `http.destroy`)는 동일.

5) **검증**  
   - `cd c-server && make` 빌드.  
   - `./http_server` 후 `curl /`, `/health`, `/ping`, `/nope`로 상태/본문 확인, 잘못된 요청에 400 응답 확인.  
   - SIGINT로 종료 플래그 확인.

## 추가 고려(선택)
- 라우터 용량 초과 시 에러 반환/로그 추가.  
- 단순 문자열 비교 외에 prefix 매칭/메서드별 라우트 확장 가능.  
- 로깅으로 메서드·경로·응답 코드 기록.
