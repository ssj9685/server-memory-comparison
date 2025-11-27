const port = Number.parseInt(process.env.PORT ?? "8081", 10) || 8081;
const body = process.env.RESPONSE_BODY ?? "Hello from Bun server\n";
const bodyBytes = new TextEncoder().encode(body);

Bun.serve({
  port,
  fetch() {
    return new Response(bodyBytes, {
      status: 200,
      headers: {
        "Content-Type": "text/plain",
        "Content-Length": bodyBytes.byteLength.toString(),
        Connection: "close",
      },
    });
  },
});

console.log(`Bun server listening on http://127.0.0.1:${port}`);
