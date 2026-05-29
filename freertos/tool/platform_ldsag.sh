#!/bin/bash

ld_size_define=("S_256" "S_1K" "S_4K" "S_16K" "S_32K" "S_40K" "S_64K" "S_112K" "S_124K" "S_128K" "S_192K" "S_256K" "S_512K" "S_1M" "S_16M" "S_1G")
# If you want to add the definitions or refered definitions in linker_script, add name of definition in ld_extern_define.
ld_extern_define=("GDMPSRAM_BASE" "GDMPSRAM_SIZE" "DMEM_BASE" "DMEM_SIZE" "DMEM_PDMA_SIZE" "DMEM_PMDA_BASE" "DMEM_INITSTACK_SIZE" "DMEM_MIN_HEAP")

if [ $# -ne 2 ] || [ "$1" == "-h" ] || [ "$1" == "--help" ];
then
	echo -e "Usage: platform_ldsag.sh <input_file> <output_file>\n"
	exit 1
fi

defines=$(cat $1 | sed -n "s/#define\s\+\\(\w\+\\)\s\+\\([^\r\n]\+\\)/\1=\2/p")

declare -A defines_array
while IFS='=' read -r k v;do
	defines_array[$k]=$v
done <<< "$defines"

echo "" > $2
for k in "${ld_size_define[@]}"; do
	echo "DEFINE $k ${defines_array[$k]}" >> $2
done

for k in "${ld_extern_define[@]}"; do
	echo "DEFINE $k ${defines_array[$k]}" >> $2
done
