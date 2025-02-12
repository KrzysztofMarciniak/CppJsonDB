grep -rl '#include "macros.h"' src/ --include=\*.* | while read -r file ; do
    sed -i '/#include "macros.h"/d' "$file"
    echo "Removed #include \"macros.h\" from $file"
done

