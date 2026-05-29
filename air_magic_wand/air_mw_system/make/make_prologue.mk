# prologue standard things
sp              := $(sp).x
dirstack_$(sp)  := $(d)
d               := $(dir)

# Wildcard sources
SRCS_$(d)       := $(wildcard $(d)/*.c)

OBJS_$(d)       := $(SRCS_$(d):%.c=%.o)

DEPS_$(d)       := $(OBJS_$(d):%.o=%.d)

SRCS_TOTAL      := $(SRCS_TOTAL) $(SRCS_$(d))

OBJS_TOTAL      := $(OBJS_TOTAL) $(OBJS_$(d))

DEPS_TOTAL      := $(DEPS_TOTAL) $(DEPS_$(d))

