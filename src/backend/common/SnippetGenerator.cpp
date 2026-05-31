#include "backend/common/SnippetGenerator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

namespace courierman::backend {
namespace {

[[nodiscard]] QString withQuery(const RequestDefinition& request) {
    QUrl url(request.url);
    QUrlQuery query(url);
    for (const auto& pair : request.queryParameters) {
        if (pair.enabled && !pair.key.isEmpty()) {
            query.addQueryItem(pair.key, pair.value);
        }
    }
    url.setQuery(query);
    return url.toString();
}

[[nodiscard]] QString escaped(const QString& input) {
    QString output = input;
    output.replace('\\', "\\\\");
    output.replace('"', "\\\"");
    output.replace('\n', "\\n");
    return output;
}

[[nodiscard]] QString singleQuoted(const QString& input) {
    QString output = input;
    output.replace('\'', "'\\''");
    return QStringLiteral("'%1'").arg(output);
}

[[nodiscard]] QString headersCurl(const RequestDefinition& request) {
    QString output;
    for (const auto& header : request.headers) {
        if (header.enabled && !header.key.isEmpty()) {
            output += QStringLiteral(" \\\n  -H %1").arg(singleQuoted(header.key + ": " + header.value));
        }
    }
    return output;
}

[[nodiscard]] QString bodyCurl(const RequestDefinition& request) {
    if (request.body.isEmpty()) {
        return {};
    }
    return QStringLiteral(" \\\n  --data %1").arg(singleQuoted(QString::fromUtf8(request.body)));
}

[[nodiscard]] QString headerJson(const RequestDefinition& request) {
    QJsonObject object;
    for (const auto& header : request.headers) {
        if (header.enabled && !header.key.isEmpty()) {
            object.insert(header.key, header.value);
        }
    }
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

}  // namespace

core::Result<QString> SnippetGenerator::generate(const RequestDefinition& request,
                                                 const QString& language) {
    const QString lang = language.toLower();
    const QString url = withQuery(request);
    const QString method = request.method.toUpper();
    const QString body = QString::fromUtf8(request.body);
    const QString headers = headerJson(request);

    if (lang == "curl") {
        return QStringLiteral("curl -X %1 %2%3%4")
            .arg(method, singleQuoted(url), headersCurl(request), bodyCurl(request));
    }
    if (lang == "wget") {
        return QStringLiteral("wget --method=%1 --output-document=- %2%3")
            .arg(method, singleQuoted(url), body.isEmpty() ? QString{} : QStringLiteral(" --body-data=%1").arg(singleQuoted(body)));
    }
    if (lang == "httpie") {
        return QStringLiteral("http %1 %2").arg(method, singleQuoted(url));
    }
    if (lang == "javascript" || lang == "typescript" || lang == "node.js") {
        return QStringLiteral(
                   "const response = await fetch(\"%1\", {\n"
                   "  method: \"%2\",\n"
                   "  headers: %3,%4\n"
                   "});\n"
                   "console.log(await response.text());")
            .arg(escaped(url),
                 method,
                 headers,
                 body.isEmpty() ? QString{} : QStringLiteral("\n  body: \"%1\",").arg(escaped(body)));
    }
    if (lang == "python") {
        return QStringLiteral(
                   "import requests\n\n"
                   "response = requests.request(\"%1\", \"%2\", headers=%3%4)\n"
                   "print(response.text)")
            .arg(method,
                 escaped(url),
                 headers,
                 body.isEmpty() ? QString{} : QStringLiteral(", data=\"%1\"").arg(escaped(body)));
    }
    if (lang == "go") {
        return QStringLiteral(
                   "req, _ := http.NewRequest(\"%1\", \"%2\", strings.NewReader(\"%3\"))\n"
                   "_ = req\n")
            .arg(method, escaped(url), escaped(body));
    }
    if (lang == "rust") {
        return QStringLiteral(
                   "let client = reqwest::Client::new();\n"
                   "let response = client.request(reqwest::Method::%1, \"%2\").body(\"%3\").send().await?;")
            .arg(method, escaped(url), escaped(body));
    }
    if (lang == "php") {
        return QStringLiteral("$response = file_get_contents(\"%1\");").arg(escaped(url));
    }
    if (lang == "java") {
        return QStringLiteral(
                   "HttpRequest request = HttpRequest.newBuilder()\n"
                   "    .uri(URI.create(\"%1\"))\n"
                   "    .method(\"%2\", HttpRequest.BodyPublishers.ofString(\"%3\"))\n"
                   "    .build();\n"
                   "HttpResponse<String> response = HttpClient.newHttpClient().send(request, HttpResponse.BodyHandlers.ofString());")
            .arg(escaped(url), method, escaped(body));
    }
    if (lang == "kotlin") {
        return QStringLiteral(
                   "val request = HttpRequest.newBuilder()\n"
                   "    .uri(URI.create(\"%1\"))\n"
                   "    .method(\"%2\", HttpRequest.BodyPublishers.ofString(\"%3\"))\n"
                   "    .build()")
            .arg(escaped(url), method, escaped(body));
    }
    if (lang == "c#") {
        return QStringLiteral(
                   "using var client = new HttpClient();\n"
                   "using var request = new HttpRequestMessage(HttpMethod.%1, \"%2\");\n"
                   "request.Content = new StringContent(\"%3\");\n"
                   "var response = await client.SendAsync(request);")
            .arg(method.at(0) + method.mid(1).toLower(), escaped(url), escaped(body));
    }
    if (lang == "swift") {
        return QStringLiteral(
                   "var request = URLRequest(url: URL(string: \"%1\")!)\n"
                   "request.httpMethod = \"%2\"\n"
                   "request.httpBody = \"%3\".data(using: .utf8)\n"
                   "let (data, response) = try await URLSession.shared.data(for: request)")
            .arg(escaped(url), method, escaped(body));
    }
    if (lang == "dart") {
        return QStringLiteral(
                   "final response = await http.%1(Uri.parse('%2'), body: '%3');")
            .arg(method.toLower(), escaped(url), escaped(body));
    }
    if (lang == "zig") {
        return QStringLiteral(
                   "var client = std.http.Client{ .allocator = allocator };\n"
                   "defer client.deinit();\n"
                   "var response = try client.fetch(.{ .method = .%1, .location = .{ .url = \"%2\" } });")
            .arg(method, escaped(url));
    }
    if (lang == "c") {
        return QStringLiteral(
                   "CURL *curl = curl_easy_init();\n"
                   "curl_easy_setopt(curl, CURLOPT_URL, \"%1\");\n"
                   "curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, \"%2\");\n"
                   "curl_easy_setopt(curl, CURLOPT_POSTFIELDS, \"%3\");\n"
                   "curl_easy_perform(curl);\n"
                   "curl_easy_cleanup(curl);")
            .arg(escaped(url), method, escaped(body));
    }
    if (lang == "c++") {
        return QStringLiteral(
                   "cpr::Response response = cpr::CustomMethod{\n"
                   "    cpr::Url{\"%1\"}, cpr::Body{\"%2\"}, cpr::Header{{\"Accept\", \"application/json\"}}, \"%3\"};")
            .arg(escaped(url), escaped(body), method);
    }

    return std::unexpected(QStringLiteral("Unsupported snippet language: %1").arg(language));
}

}  // namespace courierman::backend
