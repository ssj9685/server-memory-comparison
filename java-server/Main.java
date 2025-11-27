import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class Main {
    private static final String DEFAULT_BODY = "Hello from Java server\n";
    private static final int DEFAULT_PORT = 8082;

    public static void main(String[] args) throws Exception {
        int port = parsePort(System.getenv("PORT"), DEFAULT_PORT);
        String body = getEnvOrDefault("RESPONSE_BODY", DEFAULT_BODY);
        byte[] bodyBytes = body.getBytes(StandardCharsets.UTF_8);

        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", port), 0);
        ExecutorService executor =
                Executors.newFixedThreadPool(Math.max(2, Runtime.getRuntime().availableProcessors()));
        server.setExecutor(executor);
        server.createContext("/", new FixedResponseHandler(bodyBytes));

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            server.stop(0);
            executor.shutdown();
        }));

        server.start();
        System.out.printf("Java server listening on http://127.0.0.1:%d%n", port);

        // Keep the main thread alive until interrupted.
        Thread.currentThread().join();
    }

    private static int parsePort(String value, int fallback) {
        if (value == null || value.isEmpty()) {
            return fallback;
        }
        try {
            int parsed = Integer.parseInt(value, 10);
            if (parsed > 0 && parsed <= 65535) {
                return parsed;
            }
            return fallback;
        } catch (NumberFormatException ex) {
            return fallback;
        }
    }

    private static String getEnvOrDefault(String key, String fallback) {
        String val = System.getenv(key);
        return (val == null || val.isEmpty()) ? fallback : val;
    }

    private static final class FixedResponseHandler implements HttpHandler {
        private final byte[] body;

        private FixedResponseHandler(byte[] body) {
            this.body = body;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "text/plain");
            headers.set("Content-Length", Integer.toString(body.length));
            headers.set("Connection", "close");

            exchange.sendResponseHeaders(200, body.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(body);
            }
        }
    }
}
