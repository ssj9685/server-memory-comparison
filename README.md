🚀 Language Runtime Memory Comparison (C vs Bun vs Java)

서버를 동일한 환경에서 실행한 뒤, 포트 기준으로 PID를 찾고(ps + lsof)
각 프로세스의 메모리 사용량(RSS) 을 비교한 결과이다.

ps -o pid,%mem,rss,vsize,command -p $(lsof -ti :PORT)

---

📌 Result Summary

✔️ C Server
• 가장 가벼움 (~1MB)
• 추가 런타임 없음
• 네이티브 코드라 baseline이 극단적으로 낮음
• 작은 서버/임베디드 환경에 최적

---

✔️ Bun Server
• 약 24MB, JS 런타임 포함해서 매우 양호
• Node.js보다 훨씬 가벼운 런타임
• 스타트업/프로토타입/실서비스 모두 적합

---

✔️ Java Server
• 약 45MB, JVM 기준으로는 매우 낮은 baseline
• 순수 Java 애플리케이션일 때의 최소 수준
• Spring Boot(300~600MB) 대비 10배 이상 가벼움

---

🔍 Understanding VSZ

각 프로세스의 VSZ 값은 macOS가 64-bit 가상 주소 공간을 넓게 예약하기 때문에 매우 크게 보이지만,
이는 실제 메모리 사용량과 무관하며 RSS(실제 물리 메모리) 만 보면 된다.

---

🏁 Conclusion
• 트래픽이 낮거나 메모리 절약이 최우선이면 C가 최강
• 개발 생산성과 성능의 균형을 원하면 Bun이 매우 훌륭한 선택
• Java도 런타임 기반 서버로서는 충분히 경량 수준

단순한 테스트였지만, 언어/런타임별 baseline 메모리 특성을 명확히 알 수 있다.
