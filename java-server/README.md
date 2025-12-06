# Java server

간단한 HTTP 200 응답 서버. `PORT`(기본 8082)와 `RESPONSE_BODY`를 환경 변수로 지정할 수 있습니다. 의존성은 표준 JDK만 사용합니다.

## 빌드

```bash
make -C java-server
```

## 실행

```bash
java -cp out Main
```
