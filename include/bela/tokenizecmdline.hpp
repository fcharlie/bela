//////
#ifndef BELA_TOKENIZE_CMDLINE_HPP
#define BELA_TOKENIZE_CMDLINE_HPP
#include <string_view>
#include <cstring>
#include <vector>
#include <cstdio>
#include <cstdlib>

namespace bela {
namespace cmdline_internal {
constexpr inline bool isWhitespace(wchar_t ch) {
  return ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n';
}

constexpr inline bool isWhitespaceOrNull(wchar_t ch) {
  return isWhitespace(ch) || ch == L'\0';
}
constexpr inline bool isQuote(wchar_t ch) { return ch == '\"' || ch == '\''; }
inline void TokenFill(std::vector<wchar_t> &Token, size_t n, wchar_t ch) {
  for (size_t i = 0; i < n; i++) {
    Token.push_back(ch);
  }
}
inline size_t parseBackslash(std::wstring_view Src, size_t I,
                             std::vector<wchar_t> &Token) {
  auto E = Src.size();
  int BackslashCount = 0;
  // Skip the backslashes.
  do {
    ++I;
    ++BackslashCount;
  } while (I != E && Src[I] == '\\');

  bool FollowedByDoubleQuote = (I != E && Src[I] == '"');
  if (FollowedByDoubleQuote) {
    TokenFill(Token, BackslashCount / 2, '\\');
    if (BackslashCount % 2 == 0) {
      return I - 1;
    }
    Token.push_back('"');
    return I;
  }
  TokenFill(Token, BackslashCount, '\\');
  return I - 1;
}
} // namespace cmdline_internal
class Tokenizer {
public:
  Tokenizer() = default;
  Tokenizer(const Tokenizer &) = delete;
  Tokenizer &operator=(const Tokenizer &) = delete;
  ~Tokenizer() {
    for (auto a : saver_) {
      if (a != nullptr) {
        free(a);
      }
    }
  }
  bool Tokenize(std::wstring_view cmdline, bool MarkEOLs = false);
  const wchar_t *const *Argv() const { return saver_.data(); };
  wchar_t **Argv() { return saver_.data(); }
  size_t Argc() const { return saver_.size(); }

private:
  std::vector<wchar_t *> saver_;
  void Append(const std::vector<wchar_t> &Token);
};

void Tokenizer::Append(const std::vector<wchar_t> &Token) {
  auto size = Token.size() + 1;
  auto mem = static_cast<wchar_t *>(malloc(size * sizeof(wchar_t)));
  if (mem != nullptr) {
    memcpy(mem, Token.data(), Token.size() * sizeof(wchar_t));
    mem[Token.size()] = L'\0';
    saver_.push_back(mem);
  }
}

inline bool Tokenizer::Tokenize(std::wstring_view Src, bool MarkEOLs) {
  std::vector<wchar_t> Token(128);

  // This is a small state machine to consume characters until it reaches the
  // end of the source string.
  enum { INIT, UNQUOTED, QUOTED } State = INIT;
  for (size_t I = 0, E = Src.size(); I != E; ++I) {
    char C = Src[I];

    // INIT state indicates that the current input index is at the start of
    // the string or between tokens.
    if (State == INIT) {
      if (cmdline_internal::isWhitespaceOrNull(C)) {
        // Mark the end of lines in response files
        if (MarkEOLs && C == '\n') {
          saver_.push_back(nullptr);
        }
        continue;
      }
      if (C == '"') {
        State = QUOTED;
        continue;
      }
      if (C == '\\') {
        I = cmdline_internal::parseBackslash(Src, I, Token);
        State = UNQUOTED;
        continue;
      }
      Token.push_back(C);
      State = UNQUOTED;
      continue;
    }

    // UNQUOTED state means that it's reading a token not quoted by double
    // quotes.
    if (State == UNQUOTED) {
      // Whitespace means the end of the token.
      if (cmdline_internal::isWhitespaceOrNull(C)) {
        Append(Token);
        Token.clear();
        State = INIT;
        // Mark the end of lines in response files
        if (MarkEOLs && C == '\n') {
          saver_.push_back(nullptr);
        }
        continue;
      }
      if (C == '"') {
        State = QUOTED;
        continue;
      }
      if (C == '\\') {
        I = cmdline_internal::parseBackslash(Src, I, Token);
        continue;
      }
      Token.push_back(C);
      continue;
    }

    // QUOTED state means that it's reading a token quoted by double quotes.
    if (State == QUOTED) {
      if (C == '"') {
        if (I < (E - 1) && Src[I + 1] == '"') {
          // Consecutive double-quotes inside a quoted string implies one
          // double-quote.
          Token.push_back('"');
          I = I + 1;
          continue;
        }
        State = UNQUOTED;
        continue;
      }
      if (C == '\\') {
        I = cmdline_internal::parseBackslash(Src, I, Token);
        continue;
      }
      Token.push_back(C);
    }
  }
  // Append the last token after hitting EOF with no whitespace.
  if (!Token.empty()) {
    Append(Token);
  }
  // Mark the end of response files
  if (MarkEOLs) {
    saver_.push_back(nullptr);
  }
  return true;
}

} // namespace bela

#endif
