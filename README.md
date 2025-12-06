🚀 Language Runtime Memory Comparison (C vs Bun vs Java vs Rust)

동일한 환경에서 서버를 실행하고, 포트로 PID를 찾은 뒤 `ps` 로 RSS(실제 메모리) 를 측정했다.

```bash
ps -o pid,%mem,rss,vsize,command -p "$(lsof -ti :PORT)"
```

---

📌 Quick Stats (macOS 측정값)

| Server | Port | RSS | Notes |
| --- | --- | --- | --- |
| C | 8080 | ~1MB | 추가 런타임 없음, 네이티브 |
| Bun | 8081 | ~24MB | JS 런타임 포함, Node.js 대비 경량 |
| Java | 8082 | ~45MB | JVM 최소 수준, Spring Boot 대비 10배 이상 가벼움 |
| Rust | 8083 | 1184KB (~1.2MB) | 표준 라이브러리만 사용한 단일 바이너리 |

---

💡 Observations
- 네이티브(C, Rust)는 1MB 안팎으로 극단적으로 낮은 baseline을 보여준다.
- Bun은 JS 런타임을 포함하고도 20MB대 초반으로, 생산성과 경량성을 모두 확보한다.
- 순수 Java(JDK 기본 HTTP 서버)는 JVM을 감안하면 매우 작은 40MB대 초반에 머문다.

---

🔍 VSZ 참고
macOS는 64-bit 주소 공간을 넓게 예약해 VSZ가 크게 보인다. 실제 물리 메모리 비교에는 RSS만 보면 된다.

---

🏁 Takeaways
- 메모리 절약 최우선: C 또는 Rust
- 생산성/경량성 균형: Bun
- JVM 기반에서 최소 메모리로: 표준 Java 서버

세부 빌드·실행 방법은 각 언어별 폴더의 README를 참조한다.
