for file in $(grep -rl "checkUserAndDatabase" src/ --include=\*.cpp); do
    base_name=$(basename "$file" .cpp)
    header_file="$(dirname "$file")/headers/${base_name}.h"

    if [ -f "$header_file" ] && ! grep -q '#include "checks.h"' "$header_file"; then
        sed -i '1i#include "checks.h"' "$header_file"
        echo "Added #include \"checks.h\" to $header_file"
    fi
done
