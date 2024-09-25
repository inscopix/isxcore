set (JSON_HEADER_SEARCH_PATHS)
get_filename_component(t ${THIRD_PARTY_DIR}/json_modern_C++/2.0.1_isx ABSOLUTE)
list(APPEND JSON_HEADER_SEARCH_PATHS ${t})
