#include "webcc/utility.h"

#include <ctime>
#include <sstream>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"

#include "webcc/version.h"

namespace bfs = boost::filesystem;

namespace webcc {
namespace utility {

std::string RandomUuid() {
  boost::uuids::uuid u = boost::uuids::random_generator()();
  std::stringstream ss;
  ss << u;
  return ss.str();
}

const std::string& UserAgent() {
  static auto s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

std::string HttpDate() {
  std::time_t t = std::time(nullptr);
  tm* gmt = std::gmtime(&t);

  // Either put_time() or strftime() could format the date as expected, but they
  // are both locale dependent!
  //
  //   std::stringstream ss;
  //   ss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S") << " GMT";
  //   return ss.str();
  // 
  //   char buf[26];
  //   std::strftime(buf, 26, "%a, %d %b %Y %H:%M:%S", gmt);

  static const char* const kDays[7] = { "Sun", "Mon", "Tue", "Wed",
                                        "Thu", "Fri", "Sat" };

  static const char* const kMonths[12] = { "Jan", "Feb", "Mar", "Apr",
                                           "May", "Jun", "Jul", "Aug",
                                           "Sep", "Oct", "Nov", "Dec" };

  char buf[26];

  std::snprintf(buf, 26, "%s, %.2i %s %i %.2i:%.2i:%.2i", kDays[gmt->tm_wday],
                gmt->tm_mday, kMonths[gmt->tm_mon], gmt->tm_year + 1900,
                gmt->tm_hour, gmt->tm_min, gmt->tm_sec);

  return std::string(buf) + " GMT";
}

bool SplitKV(const std::string& str, char delimiter, std::string* key,
             std::string* value, bool trim) {
  std::size_t pos = str.find(delimiter);
  if (pos == std::string::npos) {
    return false;
  }

  *key = str.substr(0, pos);
  *value = str.substr(pos + 1);

  if (trim) {
    boost::trim(*key);
    boost::trim(*value);
  }

  return true;
}

bool ToSize(const std::string& str, int base, std::size_t* size) {
  try {
    *size = static_cast<std::size_t>(std::stoul(str, 0, base));
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

std::size_t TellSize(const Path& path) {
  // Flag "ate": seek to the end of stream immediately after open.
  bfs::ifstream stream{ path, std::ios::binary | std::ios::ate };
  if (stream.fail()) {
    return kInvalidLength;
  }
  return static_cast<std::size_t>(stream.tellg());
}

bool ReadFile(const Path& path, std::string* output) {
  // Flag "ate": seek to the end of stream immediately after open.
  bfs::ifstream stream{ path, std::ios::binary | std::ios::ate };
  if (stream.fail()) {
    return false;
  }

  auto size = stream.tellg();
  output->resize(static_cast<std::size_t>(size), '\0');
  stream.seekg(std::ios::beg);
  stream.read(&(*output)[0], size);
  if (stream.fail()) {
    return false;
  }
  return true;
}

void DumpByLine(const std::string& data, std::ostream& os,
                const std::string& prefix) {
  std::vector<std::string> lines;
  boost::split(lines, data, boost::is_any_of("\n"));

  std::size_t size = 0;

  for (const std::string& line : lines) {
    os << prefix;

    if (line.size() + size > kMaxDumpSize) {
      os.write(line.c_str(), kMaxDumpSize - size);
      os << "..." << std::endl;
      break;
    } else {
      os << line << std::endl;
      size += line.size();
    }
  }
}

}  // namespace utility
}  // namespace webcc
