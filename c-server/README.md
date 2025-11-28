# C HTTP 서버

간단한 단일 스레드 블로킹 HTTP 서버로, 환경 변수로 본문과 포트를 지정할 수 있습니다.

## 파일 구성
- `server.c`: 엔트리 포인트. 설정 로드 → 응답 준비 → 리스닝 소켓 생성 → 요청 처리 루프를 담당합니다.
- `net.h`/`net.c`: 포트 파싱, 리스닝 소켓 생성/정리, 소켓 전송 헬퍼 등 공용 네트워크 유틸리티. `create_net_context`로 `Listener`와 `NetOps`를 함께 제공합니다.
- `http.h`/`http.c`: 정적 HTTP 응답 생성과 해제.
- `signals.h`/`signals.c`: 종료 신호(SIGHUP 제외) 설치와 SIGPIPE 무시를 담당하며, 기본 종료 플래그를 내장한 `SignalOps`를 제공합니다.
- `Makefile`: `http_server` 실행 파일을 빌드합니다.
- `REFACTORING.md`: 리팩터링 근거와 적용한 패턴 설명.

## 빌드
```sh
cd c-server
make        # http_server 바이너리 생성
make clean  # 빌드 산출물 삭제
```

## 실행/설정
- `PORT`: 바인딩할 포트 (기본값 `8080`)
- `RESPONSE_BODY`: 응답 본문 (기본값 `Hello from C server\n`)

예시:
```sh
PORT=9090 RESPONSE_BODY="Hi\n" ./http_server
```

## 동작 개요
1. `SignalOps(create_signal_ops)`로 생성한 뒤 `install`로 SIGINT/SIGTERM을 받아 루프를 종료하며, SIGPIPE는 무시합니다. 종료 플래그는 ops 내부 기본 플래그를 사용하며 `get_flag`로 접근합니다. `SMC_ARRAY_LEN`은 `signals.c` 내부에서 정의 후 바로 `#undef`하여 외부 충돌을 방지합니다.
2. `HttpContext`가 내부 `HttpOps`로 응답을 한 번만 만들어 `HttpResponse`를 재사용합니다(`create_http_context`/`destroy_http_context`).
3. `ServerConfig`와 `create_net_context`로 준비한 `Listener`/`NetOps`를 사용해 `SO_REUSEADDR` 적용 후 `bind`/`listen`으로 대기하며, 포트 파싱은 `NetOps.parse_port`로 처리합니다.
4. `accept` → 요청 페이로드를 버퍼로 한 번 읽어 폐기 → 준비된 응답을 `NetOps.send_all`로 전송 → 소켓 종료 순으로 처리합니다.

## 제약/참고
- HTTP 파서를 두지 않고, 커넥션당 한 번의 요청/응답만 처리합니다.
- 블로킹 I/O 기반이므로 동시 접속이 많을 때는 스레드/이벤트 루프 추가가 필요합니다.
