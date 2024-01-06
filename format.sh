#!/bin/bash 
#
# format source code script
#

set -e

cur_dir=$(dirname $0)
dir=(src test)
pattern=(*.cc *.h)

cd ${cur_dir}
for d in ${dir[@]}; do
	for p in ${pattern[@]};do
		for file in $(find ${d} -name "$p"); do
			echo "formatting $file"	
			clang-format -style=file -i $file
		done
	done
done
