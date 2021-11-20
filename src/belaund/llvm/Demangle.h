//===--- Demangle.h ---------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEMANGLE_DEMANGLE_H
#define LLVM_DEMANGLE_DEMANGLE_H

#include <cstddef>
#include <string>

namespace llvm {
/// This is a llvm local version of __cxa_demangle. Other than the name and
/// being in the llvm namespace it is identical.
///
/// The mangled_name is demangled into buf and returned. If the buffer is not
/// large enough, realloc is used to expand it.
///
/// The *status will be set to a value from the following enumeration
enum : int {
  demangle_unknown_error = -4,
  demangle_invalid_args = -3,
  demangle_invalid_mangled_name = -2,
  demangle_memory_alloc_failure = -1,
  demangle_success = 0,
};

char *itaniumDemangle(const std::string_view mangled_name, char *buf, size_t *n,
                      int *status);

enum MSDemangleFlags {
  MSDF_None = 0,
  MSDF_DumpBackrefs = 1 << 0,
  MSDF_NoAccessSpecifier = 1 << 1,
  MSDF_NoCallingConvention = 1 << 2,
  MSDF_NoReturnType = 1 << 3,
  MSDF_NoMemberType = 1 << 4,
  MSDF_NoVariableType = 1 << 5,
};

/// Demangles the Microsoft symbol pointed at by mangled_name and returns it.
/// Returns a pointer to the start of a null-terminated demangled string on
/// success, or nullptr on error.
/// If n_read is non-null and demangling was successful, it receives how many
/// bytes of the input string were consumed.
/// buf can point to a *n_buf bytes large buffer where the demangled name is
/// stored. If the buffer is too small, it is grown with realloc(). If buf is
/// nullptr, then this malloc()s memory for the result.
/// *n_buf stores the size of buf on input if buf is non-nullptr, and it
/// receives the size of the demangled string on output if n_buf is not nullptr.
/// status receives one of the demangle_ enum entries above if it's not nullptr.
/// Flags controls various details of the demangled representation.
char *microsoftDemangle(const std::string_view mangled_name, size_t *n_read,
                        char *buf, size_t *n_buf,
                        int *status, MSDemangleFlags Flags = MSDF_None);

// Demangles a Rust v0 mangled symbol. The API follows that of __cxa_demangle.
char *rustDemangle(const std::string_view MangledName, char *Buf, size_t *N, int *Status);

// Demangles a D mangled symbol.
char *dlangDemangle(const std::string_view MangledName);

/// Attempt to demangle a string using different demangling schemes.
/// The function uses heuristics to determine which demangling scheme to use.
/// \param MangledName - reference to string to demangle.
/// \returns - the demangled string, or a copy of the input string if no
/// demangling occurred.
std::string demangle(const std::string_view MangledName);

bool nonMicrosoftDemangle(const std::string_view MangledName,std::string &Result);

/// "Partial" demangler. This supports demangling a string into an AST
/// (typically an intermediate stage in itaniumDemangle) and querying certain
/// properties or partially printing the demangled name.
struct ItaniumPartialDemangler {
  ItaniumPartialDemangler();

  ItaniumPartialDemangler(ItaniumPartialDemangler &&Other);
  ItaniumPartialDemangler &operator=(ItaniumPartialDemangler &&Other);

  /// Demangle into an AST. Subsequent calls to the rest of the member functions
  /// implicitly operate on the AST this produces.
  /// \return true on error, false otherwise
  bool partialDemangle(const char *MangledName);

  /// Just print the entire mangled name into Buf. Buf and N behave like the
  /// second and third parameters to itaniumDemangle.
  char *finishDemangle(char *Buf, size_t *N) const;

  /// Get the base name of a function. This doesn't include trailing template
  /// arguments, ie for "a::b<int>" this function returns "b".
  char *getFunctionBaseName(char *Buf, size_t *N) const;

  /// Get the context name for a function. For "a::b::c", this function returns
  /// "a::b".
  char *getFunctionDeclContextName(char *Buf, size_t *N) const;

  /// Get the entire name of this function.
  char *getFunctionName(char *Buf, size_t *N) const;

  /// Get the parameters for this function.
  char *getFunctionParameters(char *Buf, size_t *N) const;
  char *getFunctionReturnType(char *Buf, size_t *N) const;

  /// If this function has any any cv or reference qualifiers. These imply that
  /// the function is a non-static member function.
  bool hasFunctionQualifiers() const;

  /// If this symbol describes a constructor or destructor.
  bool isCtorOrDtor() const;

  /// If this symbol describes a function.
  bool isFunction() const;

  /// If this symbol describes a variable.
  bool isData() const;

  /// If this symbol is a <special-name>. These are generally implicitly
  /// generated by the implementation, such as vtables and typeinfo names.
  bool isSpecialName() const;

  ~ItaniumPartialDemangler();

private:
  void *RootNode;
  void *Context;
};
} // namespace llvm

#endif
