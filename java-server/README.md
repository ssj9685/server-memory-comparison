# Java server

간단한 HTTP 200 응답 서버. `PORT`(기본 8082)와 `RESPONSE_BODY`를 환경 변수로 지정할 수 있습니다. 의존성은 표준 JDK만 사용합니다.

## 빌드
```bash
make -C java-server
```

## 실행
```bash
PORT=18080 RESPONSE_BODY="Hello from Java" java -cp java-server/out Main
```

## 수동 RSS 스냅샷 예시
1) 서버 실행 후 PID 확인:
```bash
pgrep -f "java .*Main"
```
2) 짧은 간격으로 RSS 기록:
```bash
while true; do date +%s,; ps -o pid,rss,command -p <PID>; sleep 1; done
```
3) 완료 후 `kill -TERM <PID>`로 종료.
