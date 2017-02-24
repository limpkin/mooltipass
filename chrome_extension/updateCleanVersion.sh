#!/bin/bash
readonly CWD="$(cd "$(dirname "$0")" && pwd)"
readonly OUTPUT_DIR="${CWD}/CleanVersion"
readonly FIREFOX_TARGET='firefox'
readonly CHROMIUM_TARGET='chromium'

EXTENSION_NAME='mooltipass-extension'

BUILD_FIREFOX=0
BUILD_CHROMIUM=0

declare -A BUILD_METADATA

trap _clean_wd INT TERM
trap _clean_chrome EXIT

# main
#
# $@: argv
function main()
{
    local -i test_only
    local chrome_sign_key
    local cur_target

    test_only=0
    chrome_sign_key=''

    while [ $# -gt 0 ]; do
        case "$1" in
            '--extension-name')
                if [ $# -lt 2 ]; then
                    _usage "$0" "must proviate the name of the generated extension"
                fi
                EXTENSION_NAME="$2"
                shift 2
                ;;
             '--target')
                 if [ $# -lt 4 ]; then
                     _usage "$0" "must provide a target and the signature key file"
                 fi

                 case "$2" in
                     "$FIREFOX_TARGET")
                         :
                         BUILD_FIREFOX=1
                         cur_target="$FIREFOX_TARGET"
                         ;;
                     'chrome'|"$CHROMIUM_TARGET")
                         BUILD_CHROMIUM=1
                         cur_target="$CHROMIUM_TARGET"
                         ;;
                     *)
                         _usage "$0" "unknown target $2"
                         ;;
                 esac

                 if [ "$3" != '--sign-key' ]; then
                     _usage "$0" "must prorivde the key signature file for target $cur_target"
                 fi

                 BUILD_METADATA[${cur_target}]="$4"

                 shift 4
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
    _test
    [ "$test_only" == 0 ] && _build
}

# create the tarball to send to the store
function _build()
{
    local zip_file="${CWD}/${EXTENSION_NAME}.zip"

    if [ -f "${zip_file}" ]; then
        echo "[INFO] ${zip_file} already exist will overwrite"
    fi

    (
        cd "${OUTPUT_DIR}" || exit 1
        if ! zip -1 -qr "${zip_file}" ./*; then
            echo "[ERROR] ${zip_file} generation failed"
            exit 1
        fi
    )

    [ "$BUILD_FIREFOX" == 1 ] &&_build_firefox_xpi "${zip_file}" "${BUILD_METADATA[${FIREFOX_TARGET}]}"
    [ "$BUILD_CHROMIUM" == 1 ] &&_build_chromium_crx "${zip_file}" "${BUILD_METADATA[${CHROMIUM_TARGET}]}"
}

# build the Chromium CRX file from the generated ZIP file.
#
# following code is extracted from https://developer.chrome.com/extensions/crx
#
# $1: base ZIP file
# $2: path to private key for signin Chromium extensions
function _build_chromium_crx()
{
    local base_zip
    local zip_file
    local chrome_sign_key
    local crx_file
    # temporary generated file
    local sig
    local pub
    local pub_len_hex
    local sig_len_hex

    readonly CRMAGIC_HEX="4372 3234" # Cr24
    readonly VERSION_HEX="0200 0000" # 2

    zip_file="$1"
    chrome_sign_key="$2"
    base_zip="$(basename "${zip_file}")"
    crx_file="${base_zip//.zip/.crx}"
    sig="${base_zip//.zip/.sig}"
    pub="${base_zip//.zip/.pub}"

    _file_present "${zip_file}"
    _file_present "${chrome_sign_key}"

    # signature
    openssl sha1 -sha1 -binary -sign "${chrome_sign_key}" < "${zip_file}" > "${sig}"

    # public key
    openssl rsa -pubout -outform DER < "${chrome_sign_key}" > "${pub}" 2>/dev/null

    pub_len_hex=$(_byte_swap "$(printf '%08x\n' "$(ls -l "$pub" | awk '{print $5}')")")
    sig_len_hex=$(_byte_swap "$(printf '%08x\n' "$(ls -l "$sig" | awk '{print $5}')")")

    (
        echo "${CRMAGIC_HEX} ${VERSION_HEX} ${pub_len_hex} ${sig_len_hex}" | xxd -r -p
        cat "${pub}" "${sig}" "${zip_file}"
    ) > "${crx_file}"
}

# build the Firefox XPI from ${OUTPUT_DIR}
#
# The XPI file is expected to be a symlink to the ZIP file
#
# $1: pathname of the base zip file
# $2: path to private key for signin Firefox extension
function _build_firefox_xpi()
{
    local dir_zip
    local base_zip
    local zip_file
    local xpi_file
    local firefox_sign_key

    zip_file="$1"
    firefox_sign_key="$2"

    _file_present "${zip_file}"
    _file_present "${firefox_sign_key}"

    dir_zip="$(dirname "${zip_file}")"
    base_zip="$(basename "${zip_file}")"
    xpi_file="${base_zip//.zip/-unsigned.xpi}"
    (
        cd "${dir_zip}" || exit 1
        if ! ln -fs "${base_zip}" "${xpi_file}"; then
            echo "[ERROR] Cannot generate link ${xpi_file} to ${base_zip}"
            exit 1
        fi
    )
}

# Swaping algorithm for CRX
#
# Take "abcdefgh" and return it as "ghefcdab"
#
# $1: input byte stream
function _byte_swap()
{
    echo "${1:6:2}${1:4:2}${1:2:2}${1:0:2}"
}


# assert that the given file is present on FS
#
# $1: file path
function _file_present()
{
    local f

    f="${1}"

    if [ ! -f "${f}" ]; then
        echo "[ERROR] ${f} does not exists" 1>&2
        exit 2
    fi
}

# clean temp file from chrome build
function _clean_chrome()
{
    rm -f "${CWD}/${EXTENSION_NAME}.sig"        \
       "${CWD}/${EXTENSION_NAME}.pub"
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

# performs a list of tests on the extension archive directory
function _test()
{
    if [ -f "${OUTPUT_DIR}/install.rdf" ] && [ -f "${OUTPUT_DIR}/manifest.json" ]; then
        echo "[ERROR] Both an install.rdf and manifest.json are provided" 1>&2
        exit 1
    fi

    _test_firefox
}

# specific tests for Firefox
function _test_firefox()
{
    if [ -f "${OUTPUT_DIR}/install.rdf" ]; then
        echo "[ERROR] The Firefox store will handle this extension as add-on"
        exit 3
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
Usage: $prog_name [--extension-name NAME] [ TARGET ] [--test]
where TARGET := --target {chrome | chromium | firefox} --sign-key SIGN_KEY

      --extension-name  name of the generated extension files
      --sign-key        path to signature key file for the specific target (ie: Chromium, Firefox...)
      --target          create a clean directory for the given target chromium(default)
      --test            only perform test, no packaging is performed
EOF

    exit 1
}

if [ "${BASH_SOURCE[0]}" = "$0" ]; then
    main "$@"
fi
