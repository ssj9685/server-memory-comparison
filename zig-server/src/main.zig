const std = @import("std");

const DEFAULT_PORT: u16 = 8084;
const DEFAULT_BODY: []const u8 = "Hello from Zig server\n";

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const port = try getPort(allocator);
    const body = try getEnvOrDefault(allocator, "RESPONSE_BODY", DEFAULT_BODY);
    defer allocator.free(body);

    const response = try buildResponse(allocator, body);
    defer allocator.free(response);

    const address = try std.net.Address.parseIp4("0.0.0.0", port);
    var server = try address.listen(std.net.Address.ListenOptions{});
    defer server.deinit();

    std.debug.print("Zig server listening on http://127.0.0.1:{d}\n", .{port});

    var buf: [1024]u8 = undefined;
    while (true) {
        const conn = server.accept() catch |err| {
            std.log.err("accept error: {}", .{err});
            continue;
        };

        var stream = conn.stream;
        _ = stream.read(&buf) catch {};
        stream.writeAll(response) catch |err| {
            std.log.err("write error: {}", .{err});
        };
        stream.close();
    }
}

fn getPort(allocator: std.mem.Allocator) !u16 {
    const env_val = std.process.getEnvVarOwned(allocator, "PORT") catch return DEFAULT_PORT;
    defer allocator.free(env_val);

    const parsed = std.fmt.parseInt(u16, env_val, 10) catch return DEFAULT_PORT;
    return if (parsed == 0) DEFAULT_PORT else parsed;
}

fn getEnvOrDefault(allocator: std.mem.Allocator, key: []const u8, fallback: []const u8) ![]u8 {
    return std.process.getEnvVarOwned(allocator, key) catch allocator.dupe(u8, fallback);
}

fn buildResponse(allocator: std.mem.Allocator, body: []const u8) ![]u8 {
    return try std.fmt.allocPrint(
        allocator,
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {d}\r\nConnection: close\r\n\r\n{s}",
        .{ body.len, body },
    );
}
