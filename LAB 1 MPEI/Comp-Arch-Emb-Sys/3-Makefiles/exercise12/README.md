# Parallel-friendly with order-only prerequisites & .PHONY hygiene

Make the project safe for make -j parallel builds. Ensure directories are created exactly once with order-only prerequisites (|). Add a .PHONY format target that runs a formatter (e.g., clang-format) without interfering with the DAG.
