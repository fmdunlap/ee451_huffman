#!/usr/bin/env bash

( [[ -n ${ZSH_EVAL_CONTEXT} && ${ZSH_EVAL_CONTEXT} =~ :file$ ]] || \
  [[ -n $BASH_VERSION && $0 != "$BASH_SOURCE" ]]) && _s_=1 || _s_=0

export _s_
export ProjectRoot=$(pwd)
export BuildDir="${ProjectRoot}/build/run"
export BashLibRoot="${ProjectRoot}/bin/lib-bash"
export LibBashRepo="https://github.com/kigster/lib-bash"

export DEFAULT_N_PROCS=4
export DEFAULT_TYPE=s
export DEFAULT_OUTPUT_FILE=./out

# We are using an awesome BASH library `lib-bash` for prettifying the output, and
# running commands through their LibRun framework.
huffman::lib-bash() {
  if [ ! -d "./.lib-bash" ]; then 
    git clone https://github.com/pioneerworks/lib-bash .lib-bash
  fi
  source .lib-bash/lib/Loader.bash

  run::set-all show-output-off abort-on-error
}

huffman::parseargs(){
    OPTIONS=n:t:o:s
    LONGOPTS=numprocs,type,output,srun
    
    PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")

    eval set -- "$PARSED"

    n=$DEFAULT_N_PROCS
    t=$DEFAULT_TYPE
    o=$DEFAULT_OUTPUT_FILE
    s=n
    inputFile=""

    while true; do
        case "$1" in
            -n|--numprocs)
                n="$2"
                shift 2
                ;;
            -t|--type)
                #Use tr to force lowercase
                t=$(echo "$2" | tr '[:upper:]' '[:lower:]')
                shift 2
                ;;
            -o|--output)
                o="$2"
                shift 2
                ;;
            -s|--srun)
                s=y
                shift
                ;;
            --)
                shift
                break
                ;;
            *)
                inf "Programming error"
                not_ok:
                exit 3
                ;;
        esac
    done

    # handle non-option arguments
    if [[ $# -ne 1 ]]; then
        inf "$0: A single input file is required."
        not_ok:
        exit 4
    else
        inputFile=$1
    fi

}

huffman::run() {
    huffman::parseargs $@
    h1 "Running..."
    info "Input File: $inputFile"
    info "Output File: $o"
    info "Number of Processors: $n"
    info "Execution type: $t"
    info "Use srun? $s"

    if [ $s == "y" ]; then
        executable="srun"
        procarg="-n"
    else
        executable="mpirun"
        procarg="-np"
    fi


    if [ $t == "s" ] || [ $t == "serial" ]
    then
        h2 "RUN SERIAL VERSION"
        run "$executable $procarg 1 ./bin/huffman s $inputFile > $o"
    elif [ $t == "p" ] || [ $t == "parallel" ]
    then
        h2 "RUN PARALLEL VERSION"
        run "$executable $procarg $n ./bin/huffman p $inputFile > $o"
    elif [ $t == "d" ] || [ $t == "debug" ]
    then
        h2 "RUNNING DEBUG VERSION"
        mpirun $procarg $n ./bin/huffman s $inputFile
    else
        h2 "TYPE DOES NOT EXIST"
    fi
}

main() {
  huffman::lib-bash
  huffman::run $@
}

(( $_s_ )) || main $@