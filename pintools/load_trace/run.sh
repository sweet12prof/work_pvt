#!/bin/bash
if [ ! -d "${TRACE_DIR}" ]; then
    echo 'set the trace directory'
    exit 0
fi 

if [ ! -f "${PINTOOL_LIB}" ]; then
    echo 'specify pin tool lib  with PINTOOL_LIB'
    exit 0
fi 

if [ -z "${INTEL_SDE_ROOT}" ] || [ ! -d "$INTEL_SDE_ROOT" ]; then
    echo 'set the INTEL_SDE_ROOT  directory'
    exit 0
fi 


cp "${PINTOOL_LIB}" "${INTEL_SDE_ROOT}/intel64/"
TOOL=$(basename ${PINTOOL_LIB})

echo "${PINTOOL_LIB}"

rm -rf "${TRACE_DIR}/OUT_DIR"

dir_list=("$TRACE_DIR"/*)
mkdir -p "${TRACE_DIR}/OUT_DIR"

for dir in "${dir_list[@]}"; do 
    folder_name=$(basename "${dir}")
    "${INTEL_SDE_ROOT}/sde64" -replay  -replay:basename "${TRACE_DIR}/${folder_name}" \
     -replay:addr_trans   -t "${TOOL}" \
     -o  "${TRACE_DIR}/OUT_DIR/${folder_name}" -- "${INTEL_SDE_ROOT}/intel64/nullapp"
done 