#compdef specs

_specs() {
  local cur prev
  local -a completions
  
  cur="${words[CURRENT]}"
  prev="${words[CURRENT-1]}"

  # Call specs-autocomplete and capture newline-separated results
  completions=("${(@f)$(specs-autocomplete specs "$cur" "$prev")}")

  compadd -- $completions
}

_specs "$@"

