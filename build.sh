#!/usr/bin/env bash

( [[ -n ${ZSH_EVAL_CONTEXT} && ${ZSH_EVAL_CONTEXT} =~ :file$ ]] || \
  [[ -n $BASH_VERSION && $0 != "$BASH_SOURCE" ]]) && _s_=1 || _s_=0

export _s_
export ProjectRoot=$(pwd)
export BuildDir="${ProjectRoot}/build/run"
export BashLibRoot="${ProjectRoot}/bin/lib-bash"
export LibBashRepo="https://github.com/kigster/lib-bash"

# We are using an awesome BASH library `lib-bash` for prettifying the output, and
# running commands through their LibRun framework.
huffman::lib-bash() {
  [[ ! -d ${BashLibRoot} ]] && curl -fsSL https://git.io/fxZSi | /usr/bin/env bash
  [[ ! -d ${BashLibRoot} ]] && { 
    printf "Unable to git clone lib-bash repo from ${LibBashRepo}"
    exit 1
  }
  
  if [[ -f ${BashLibRoot}/Loader.bash ]]; then
    cd ${BashLibRoot} > /dev/null
    git reset --hard origin/master 2>&1 | cat >/dev/null
    git pull 2>&1 | cat >/dev/null
    [[ -f Loader.bash ]] && source Loader.bash
    cd ${ProjectRoot}
  else
    printf "\nERROR: unable to find lib-bash library from ${LibBashRepo}!\n"
    exit 1
  fi

  run::set-all show-output-off abort-on-error
}

huffman::header() {
  local OIFC=${IFC}
  IFS="|" read -r -a gcc_info <<< "$(gcc --version 2>&1 | tr '\n' '|')"
  export IFC=${OIFC}
  h1 "${bldylw}GCC" "${gcc_info[1]}" "${gcc_info[2]}" "${gcc_info[3]}" "${gcc_info[4]}"
  h1 "${bldylw}GIT:    ${bldblu}$(git --version)"
  h1 "${bldylw}CMAKE:  ${bldblu}$(cmake --version | tr '\n' ' ')"
}
# Note, I use the directory 'make' so that I can name this shell script build
# without causing too much confusion.
huffman::setup() {
  hl::subtle "Creating Build Folder..."
  run "mkdir -p make/run"

  [[ -f .idea/workspace.xml ]] || cp .idea/workspace.xml.example .idea/workspace.xml
}

huffman::clean() {
  hl::subtle "Cleaning output folders..."
  run 'rm -rf bin/d* include/d* lib/*'
}

huffman::build() {
  run "cd make/run"
  run "cmake ../.. "
  run "make -j 12"
  run "cd ${ProjectRoot}"
}

main() {
  huffman::lib-bash
  huffman::header
  huffman::setup
  huffman::build
  # Note, WE CAN AND SHOULD RUN TESTS HERE.
  # Currently, we don't have any relevant tests, soooo
  # those are things we should add in the future.
  # To run them here simply uncomment the following lines:
  # huffman::tests
  # huffman::examples
}

(( $_s_ )) || main
