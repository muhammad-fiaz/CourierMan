#include "backend/common/FeatureCatalog.h"

namespace courierman::backend {

QList<FeatureDefinition> FeatureCatalog::allFeatures() {
    return {
        {"Protocols", "REST", "HTTP request builder, auth, headers, bodies, and responses.", true, true},
        {"Protocols", "GraphQL", "Queries, variables, schema awareness, and docs.", true, false},
        {"Protocols", "WebSocket", "Streaming socket sessions and message history.", true, false},
        {"Protocols", "Socket.IO", "Event-driven Socket.IO sessions.", true, false},
        {"Protocols", "gRPC", "Reflection, proto import, unary and streaming calls.", true, false},
        {"Protocols", "SSE", "Server-sent event viewer.", true, false},
        {"Protocols", "SOAP", "Envelope builder and XML response tooling.", true, false},
        {"Protocols", "MQTT", "Broker profiles, publish, subscribe, and retained messages.", true, false},
        {"Protocols", "Raw TCP/UDP", "Low-level socket request sessions.", true, false},
        {"Workspaces", "Collections", "Nested folders, saved requests, and import/export.", true, true},
        {"Workspaces", "Environments", "Global and local variables with templating.", true, true},
        {"Workspaces", "Secret Vault", "mTLS, certificates, proxies, and protected values.", true, false},
        {"Testing", "Pre-request scripts", "JavaScript setup scripts compatible with pm APIs.", true, false},
        {"Testing", "Tests", "pm.test-style assertions and reports.", true, false},
        {"Testing", "Collection runner", "Desktop and planned headless CLI execution.", true, false},
        {"Response", "Pretty viewers", "JSON, XML, HTML, image, PDF, raw, hex, and diff views.", true, false},
        {"Code Gen", "Snippets", "20+ generated client snippets.", true, true},
        {"AI", "Cloud providers", "OpenAI, Gemini, and Claude assistance.", true, false},
        {"AI", "Local providers", "Ollama and llama.cpp offline assistance.", true, false},
        {"DevOps", "Auto updater", "GitHub Releases check and download progress.", true, true},
        {"DevOps", "Issue reporter", "Prefilled GitHub issue templates.", true, true},
        {"Platform", "System tray", "Open, settings, and exit menu.", true, true},
        {"Platform", "Autostart", "OS boot startup setting.", true, true},
    };
}

QStringList FeatureCatalog::protocolNames() {
    return {"REST", "GraphQL", "WebSocket", "Socket.IO", "gRPC", "SSE",
            "SOAP", "MQTT", "Raw TCP", "UDP"};
}

QStringList FeatureCatalog::codeGenerationLanguages() {
    return {"cURL", "Wget", "HTTPie", "C", "C++", "C#", "Java", "Kotlin",
            "Swift", "Rust", "Go", "Python", "JavaScript", "TypeScript",
            "Node.js", "PHP", "Ruby", "Dart", "Zig", "Objective-C", "Perl", "R"};
}

}  // namespace courierman::backend
