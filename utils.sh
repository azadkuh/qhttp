#!/bin/bash

CTAGS_EXCLUDES_BASE="--exclude=.git --exclude=tmp --exclude=xbin --exclude=res"
CTAGS_OPTIONS_BASE="--c++-kinds=+cefgnps --fields=+iaS --extra=+fq -R ."
CLOC_EXCLUDES_BASE="--exclude-dir=.git,xbin,tmp,res"
CLOC_EXCLUDES_LANGS="--exclude-lang=Prolog,make"


function fn_cloc() {
echo "cloc (exclusive) ..."
cloc $CLOC_EXCLUDES_LANGS $CLOC_EXCLUDES_BASE,3rdparty .
}

function fn_cloc_all() {
echo "cloc all (inclusive) ..."
cloc $CLOC_EXCLUDES_LANGS $CLOC_EXCLUDES_BASE .
}

function fn_ctags() {
echo "ctags (exclusive) ..."
ctags --exclude=3rdparty $CTAGS_EXCLUDES_BASE $CTAGS_OPTIONS_BASE
}

function fn_ctags_all() {
echo "ctags all (inclusive) ..."
ctags $CTAGS_EXCLUDES_BASE $CTAGS_OPTIONS_BASE
}

if [[ $# -eq 0 ]]; then
    fn_ctags

else
    while [[ $# > 0 ]]; do
        arg="$1"

        case $arg in
            tags|ctags|tag|ctag)
                fn_ctags
                ;;

            tags_all|ctags_all|tag_all|fn_ctags_all)
                fn_ctags_all
                ;;

            cloc_all)
                fn_cloc_all
                ;;

            cloc)
                fn_cloc
                ;;

            *)
                echo "unknown args as $arg"
                ;;

        esac
        shift
    done
fi

