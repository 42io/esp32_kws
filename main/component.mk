#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# sox input.wav -t raw raw/output.raw

COMPONENT_EMBED_FILES := raw/zero.raw \
                         raw/unk.raw \
                         raw/house.raw
