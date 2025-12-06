use std::env;
use std::io::{Read, Write};
use std::net::{Shutdown, TcpListener};

const DEFAULT_PORT: u16 = 8083;
const DEFAULT_BODY: &str = "Hello from Rust server\n";

fn main() -> std::io::Result<()> {
    let port = parse_port(env::var("PORT").ok()).unwrap_or(DEFAULT_PORT);
    let body = env::var("RESPONSE_BODY").unwrap_or_else(|_| DEFAULT_BODY.to_owned());

    let response = format!(
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
        body.len(),
        body
    );

    let listener = TcpListener::bind(("0.0.0.0", port))?;
    println!("Rust server listening on http://127.0.0.1:{port}");

    // Handle connections sequentially; good enough for the memory baseline test.
    for stream in listener.incoming() {
        let mut stream = match stream {
            Ok(s) => s,
            Err(err) => {
                eprintln!("Failed to accept connection: {err}");
                continue;
            }
        };

        let mut buffer = [0u8; 1024];
        // Best-effort request read; content is ignored because we always send the same response.
        if let Err(err) = stream.read(&mut buffer) {
            eprintln!("Failed to read request: {err}");
            continue;
        }

        if let Err(err) = stream.write_all(response.as_bytes()) {
            eprintln!("Failed to write response: {err}");
            continue;
        }
        let _ = stream.shutdown(Shutdown::Both);
    }

    Ok(())
}

fn parse_port(raw: Option<String>) -> Option<u16> {
    let candidate = raw?;
    let port: u16 = candidate.parse().ok()?;
    if port == 0 {
        None
    } else {
        Some(port)
    }
}
