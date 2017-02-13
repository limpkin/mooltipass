#!/bin/bash
readonly CWD="$(cd "$(dirname "$0")" && pwd)"
readonly OUTPUT_DIR="${CWD}/CleanVersion"

trap _clean_wd INT TERM

# main
#
# $@: argv
function main()
{
    local -i build_firefox

    build_firefox=0

    if [ $# -gt 2 ]; then
        _usage "$0"
    fi

    while [ $# -gt 0 ]; do
        case "$1" in
             '--target')
                 if [ $# -lt 2 ]; then
                     _usage "$0" "must provide a target"
                 fi

                 case "$2" in
                     'firefox')
                         build_firefox=1
                         ;;
                     *)
                         _usage "$0" "unknown target $2"
                         ;;
                 esac
                 shift 2
                 ;;
             *)
                 _usage "$0"
                 ;;
        esac
    done

    _clean_wd

    if [ ! -d "$OUTPUT_DIR" ]; then
        mkdir -p "$OUTPUT_DIR"
    fi

    _inject_scripts

    [ $build_firefox != 0 ] && _inject_firefox_scripts
}

# clean working directory
function _clean_wd()
{
    # Remove previous files
    rm -Rf "${OUTPUT_DIR:?}/"*
}

# inject extension script into OUTPUT_DIR
function _inject_scripts()
{
    # Inject Scripts
    cp "${CWD}/mooltipass-content.js" "${OUTPUT_DIR}/mooltipass-content.js"
    cp "${CWD}/mooltipass-content.css" "${OUTPUT_DIR}/mooltipass-content.css"
    cp "${CWD}/manifest.json" "${OUTPUT_DIR}/"
    cp "${CWD}/mooltipass-pre-jquery.js" "${OUTPUT_DIR}/"

    for ext_dir in vendor popups css options background images icons; do
        cp -Rf "${CWD}/${ext_dir}" "${OUTPUT_DIR}/"
    done
}

# inject scripts for Firefox
function _inject_firefox_scripts()
{
    cp "${CWD}/install.rdf" "${OUTPUT_DIR}/"
}

# print usage message on stderr
#
# $1: progname
# $*: message
function _usage()
{
    local prog_name
    prog_name="$(basename "$0")"

    shift
    if [ $# -ge 1 ]; then
        cat <<EOF 1>&2
$@
EOF
    fi

    cat <<EOF 1>&2
Usage: $prog_name [--target TARGET]
where TARGET := {chromium | firefox}

      --target     create a clean directory for the given target chromium(default)
EOF

    exit 1
}

if [ "${BASH_SOURCE[0]}" = "$0" ]; then
    main "$@"
fi
