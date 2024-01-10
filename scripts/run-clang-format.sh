find . -regex '.*\.\(c\|cpp\|cxx\|h\|hpp\|hxx\)$' -exec clang-format -i --style=file '{}' \;
