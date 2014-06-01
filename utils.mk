# Parts take from the GNU Make Standard Library
__gmsl_tr1 :=
__gmsl_tr2 :=
__gmsl_tr3 :=

# ----------------------------------------------------------------------------
# Function:  assert_no_dollar
# Arguments: 1: Name of a function being executd
#            2: Arguments to check
# Returns:   None
# ----------------------------------------------------------------------------
assert_no_dollar = $(call __gmsl_tr2)$(call assert,$(call not,$(findstring $(__gmsl_dollar),$2)),$1 called with a dollar sign in argument)

# ----------------------------------------------------------------------------
# Function:  tr
# Arguments: 1: The list of characters to translate from 
#            2: The list of characters to translate to
#            3: The text to translate
# Returns:   Returns the text after translating characters
# ----------------------------------------------------------------------------
tr = $(strip $(__gmsl_tr3)$(call assert_no_dollar,$0,$1$2$3)              \
     $(eval __gmsl_t := $3)                                               \
     $(foreach c,                                                         \
         $(join $(addsuffix :,$1),$2),                                    \
         $(eval __gmsl_t :=                                               \
             $(subst $(word 1,$(subst :, ,$c)),$(word 2,$(subst :, ,$c)), \
                 $(__gmsl_t))))$(__gmsl_t))

# Common character classes for use with the tr function.  Each of
# these is actually a variable declaration and must be wrapped with
# $() or ${} to be used.

[A-Z] := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z #
[a-z] := a b c d e f g h i j k l m n o p q r s t u v w x y z #

# ----------------------------------------------------------------------------
# Function:  uc
# Arguments: 1: Text to upper case
# Returns:   Returns the text in upper case
# ----------------------------------------------------------------------------
uc = $(__gmsl_tr1)$(call assert_no_dollar,$0,$1)$(call tr,$([a-z]),$([A-Z]),$1)
