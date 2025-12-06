# Zig server

간단한 HTTP 200 응답 서버입니다. `PORT`(기본 8084)와 `RESPONSE_BODY`를 환경 변수로 지정할 수 있습니다. 표준 라이브러리만 사용합니다.

## 빌드
```bash
cd zig-server
zig build -Doptimize=ReleaseSafe
```

## 실행
```bash
cd zig-server
PORT=18084 RESPONSE_BODY="Hello from Zig" zig build run -Doptimize=ReleaseSafe
```

## 수동 RSS 스냅샷 예시
1) 서버 실행 후 PID 확인:
```bash
pgrep -f "zig-server"
```
2) 짧은 간격으로 RSS 기록:
```bash
while true; do date +%s,; ps -o pid,rss,command -p <PID>; sleep 1; done
```
3) 완료 후 `kill -TERM <PID>`로 종료.
