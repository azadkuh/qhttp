#!/bin/bash
################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
source `dirname ${BASH_SOURCE[0]}`/helper.shi

BasePath=$1
IncludeTarget=$2
ConfigTarget=$3

# Creating a symbolic link reduces the pain of ambigious changes in headers!
#cp -vrf --parents `find $BasePath -name *.h -o -name *.hpp -o -name *.hh` $IncludeTarget || :
for File in $(find "$BasePath/" -name "*.h" -o -name "*.hpp" -o -name "*.hh"); do
    SrcPath=$(dirname $File);
    SrcName=$(basename $File);
    # Check if the header is private
    if echo "$SrcPath" | egrep "\b[P|p]rivate\b" 2>&1 > /dev/null; then
        ignore "Ignoring private header $File ..."
    else
        TgtPath="$IncludeTarget/$BasePath/$(python -c "import os.path; print(os.path.relpath('$SrcPath', '$BasePath'))")";
        SrcPath="$(python -c "import os.path; print(os.path.relpath('$SrcPath', '$TgtPath'))")";
        if [ -r "$TgtPath/$SrcName" ]; then
            ignore "Already exists $File ...";
        else
            mkdir -pv "$TgtPath" || : ;
            info "Creating symbolic link for $File ...";
            # DO NOT USE $File BECAUSE THE SYMBOLIC LINK
            # MUST POINT TO A RELATIVE PATH
            ln -s "$SrcPath/$SrcName" "$TgtPath/$SrcName" || : ;
        fi
    fi
done

echo -e "\e[32mHeaders symlinked to $IncludeTarget\e[0m"

# Config files will be copied only when the exist!
if [ -d conf ]; then
    mkdir -p $ConfigTarget    || : ;
    info "Copying config files ...";
    cp -rvf "conf/"* $ConfigTarget || : ;
    happy "Configs exported to $ConfigTarget";
else
    ignore "No config files to copy";
fi

