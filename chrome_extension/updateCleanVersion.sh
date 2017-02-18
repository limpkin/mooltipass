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
    local -i test_only

    build_firefox=0
    test_only=0

    if [ $# -gt 3 ]; then
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
             '--test')
                 test_only=1
                 shift
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

    _test "$build_firefox"

    if [ "$test_only" == 0 ]; then
        _build "$build_firefox"
    fi
}

# create the tarball to send to the store
#
# $1: boolean set if we are building Firefox add-on/web extension
function _build()
{
    local -i build_firefox

    build_firefox=$1

    case "$build_firefox" in
        1)
            _build_firefox_xpi
            ;;
        *)
            echo "[WARNING] Build for this target is not implemented"
            exit 2
            ;;
    esac
}

# build the Firefox XPI from ${OUTPUT_DIR}
function _build_firefox_xpi()
{
    local extension_id
    local xpi_file

    # the final grep is to remove the Firefox id from the list
    # I don't manage to remove it from sed :/
    extension_id='mooltipass@themooltipass'
    xpi_file="${CWD}/${extension_id}.xpi"

    if [ -f "${xpi_file}" ]; then
        echo "[INFO] ${xpi_file} already exist will overwrite"
    fi

    (
        cd "${OUTPUT_DIR}" || exit 1
        if ! zip -1 -qr "${xpi_file}" ./*; then
            echo "[ERROR] ${xpi_file} generation failed"
            exit 1
        fi
    )
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

    for ext_dir in vendor popups css options background images icons; do
        cp -Rf "${CWD}/${ext_dir}" "${OUTPUT_DIR}/"
    done
}

# inject scripts for Firefox
function _inject_firefox_scripts()
{
    if [ -f "${CWD}/install.rdf" ]; then
        echo "[WARNING] Firefox add-on is planed to be deprecated at end of 2017 consider creating WebExtensions" 1>&2
        return 1
    fi
}

# performs a list of tests on the extension archive directory
#
# $1: build_firefox
function _test()
{
    local -i build_firefox

    build_firefox=$1

    if [ -f "${OUTPUT_DIR}/install.rdf" ] && [ -f "${OUTPUT_DIR}/manifest.json" ]; then
        echo "[ERROR] Both an install.rdf and manifest.json are provided" 1>&2
        exit 1
    fi

    [ "$build_firefox" != 0 ] && _test_firefox
}

# specific tests for Firefox
function _test_firefox()
{
    if [ -f "${OUTPUT_DIR}/manifest.json" ]; then
        echo "[WARNING] The Firefox store will handle this extension as an WebExtension"
    elif [ -f "${OUTPUT_DIR}/install.rdf" ]; then
        echo "[WARNING] The Firefox store will handle this extension as add-on"
        python3 "${CWD}/tools/validate_rdf.py" --input "${OUTPUT_DIR}/install.rdf"
    fi
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
Usage: $prog_name [--target TARGET] [--test]
where TARGET := {chromium | firefox}

      --target     create a clean directory for the given target chromium(default)
      --test       only perform test, no packaging is performed
EOF

    exit 1
}

if [ "${BASH_SOURCE[0]}" = "$0" ]; then
    main "$@"
fi
