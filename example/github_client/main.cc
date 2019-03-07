#include <iostream>
#include <map>

#include "json/json.h"

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

// -----------------------------------------------------------------------------

// Change to 1 to print response JSON.
#define PRINT_RESPONSE 0

#if (defined(WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

const std::size_t kBufferSize = 1500;

// -----------------------------------------------------------------------------

static Json::Value StringToJson(const std::string& str) {
  Json::Value json;

  Json::CharReaderBuilder builder;
  std::stringstream stream(str);
  std::string errors;
  if (!Json::parseFromStream(builder, stream, &json, &errors)) {
    std::cerr << errors << std::endl;
  }

  return json;
}

// Print the JSON string in pretty format.
static void PrettyPrintJsonString(const std::string& str) {
  Json::Value json = StringToJson(str);

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  ";

  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(json, &std::cout);

  std::cout << std::endl;
}

// -----------------------------------------------------------------------------

#if PRINT_RESPONSE
#define PRINT_JSON_STRING(str) PrettyPrintJsonString(str)
#else
#define PRINT_JSON_STRING(str)
#endif  // PRINT_RESPONSE

//static void PrintError(const webcc::RestSslClient& client) {
//  std::cout << webcc::DescribeError(client.error());
//  if (client.timed_out()) {
//    std::cout << " (timed out)";
//  }
//  std::cout << std::endl;
//}
//
//// -----------------------------------------------------------------------------
//
//// List public events.
//static void ListEvents(webcc::RestSslClient& client) {
//  if (client.Get("/events")) {
//    PRINT_JSON_STRING(client.response_content());
//  } else {
//    PrintError(client);
//  }
//}
//
//// List the followers of the given user.
//static void ListUserFollowers(webcc::RestSslClient& client,
//                              const std::string& user) {
//  if (client.Get("/users/" + user + "/followers")) {
//    PRINT_JSON_STRING(client.response_content());
//  } else {
//    PrintError(client);
//  }
//}

// List the followers of the current authorized user.
// Header syntax: Authorization: <type> <credentials>
static void ListAuthorizedUserFollowers(webcc::HttpClientSession& session,
                                        const std::string& auth) {
  auto r = session.Request(webcc::HttpRequestArgs("GET").
                           url("https://api.github.com/user/followers").
                           headers({ { "Authorization", auth } }).
                           ssl_verify(kSslVerify).buffer_size(kBufferSize));

  if (r) {
    PRINT_JSON_STRING(r->content());
  } else {
    //PrintError(client);
  }
}

// -----------------------------------------------------------------------------

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::HttpClientSession session;

  //ListAuthorizedUserFollowers(client, "Basic c3ByaW5mYWxsQGdtYWlsLmNvbTpYaWFvTHVhbjFA");
  ListAuthorizedUserFollowers(session, "Token 1d42e2cce49929f2d24b1b6e96260003e5b3e1b0");

  return 0;
}