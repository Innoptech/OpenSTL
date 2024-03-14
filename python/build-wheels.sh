#!/bin/bash
set -e -u -x

function repair_wheel {
    wheel="$1"
    if ! auditwheel show "$wheel"; then
        echo "Skipping non-platform wheel $wheel"
    else
        auditwheel repair "$wheel" --plat "$PLAT" -w /io/wheelhouse/
    fi
}

# Compile wheels
for PYBIN in /opt/python/*/bin; do
    OPENSTL_SOURCE_DIR=$PWD "${PYBIN}/pip" wheel /io/ --no-deps -w wheelhouse/
done

# Bundle external shared libraries into the wheels
for whl in wheelhouse/*.whl; do
    repair_wheel "$whl"
    rm $whl
done

# Install packages and test
for PYBIN in /opt/python/*/bin/; do
    "${PYBIN}/pip" install pytest numpy
    "${PYBIN}/pip" install openstl --no-index -f /io/wheelhouse
    (cd "$HOME"; "${PYBIN}/pytest" /io/tests/python)
done