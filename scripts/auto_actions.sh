#!/usr/bin/env bash

_help_action ()
{
    # print help message
    printf $_GREEN
    cat >&1 <<EOF

    [ AUTO HELPER SCRIPT ]

    usage: ${0} [-xvs] {ACTION} [param param ...]

    ACTIONS:

    help                  show this message and exit
    build                 build the services images
    setup                 build and create services
    start                 start the services containers
    stop                  stop the services
    rm                    remove services

    exec                  execute a command in service containers
    exec_root             execute a command as root in a service container
    logs                  tail the logs of a running services

    services              list services names
    volumes               list volumes names
    images                list images information
    images-ids            list images ids
    container-ids         list containers ids

    OPTIONS:

    -x                    enables debug capability
    -s name[,name]        set target services
    -f docker-compose.yml indicates the path of the compose file
    -v                    enables verbose capability

EOF
    printf $_RESET
}

_services_action ()
{
    _compose_config --services
}

_volumes_action ()
{
    _compose_config --volumes
}

_images_action ()
{
    docker compose images `(_services_action)`
}

_images-ids_action ()
{
    docker compose images -q `(_services_action)`
}

_containers_action ()
{
    docker compose ps -a `(_services_action)`
}

_containers-ids_action ()
{
    docker compose ps -q `(_services_action)`
}

_logs_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    _compose_action "logs -f --tail 1" ${@}
}

check_cmd ()
{
    local _cmd=$@

    test -n "${_COMPOSE_SERVICE}" || \
        die "This action needs a service, run $0 help for help."

    test -n "${_cmd[@]}" || \
        die "this action needs a command, run $0 help for help."
}

_exec_action()
{
    local _cmd=$@

    # check command: mandatory argument
    check_cmd $@

    # execute the command in the chosen services
    for service in ${_COMPOSE_SERVICE}; do
        # logs
        log "Running [${_cmd[@]}] into [${service}] service\n"
        # execute as default user
        _compose_exec ${service} "$@"
    done
}

_exec_root_action()
{
    local _cmd=$@

    # check command: mandatory argument
    check_cmd $@

    # execute the command in the chosen services as root
    for service in ${_COMPOSE_SERVICE}; do
        # logs
        log "Running [${_cmd[@]}] into [${service}] service as root\n"
        # execute as default user
        _compose_root_exec ${service} "$@"
    done
}

_build_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    # build services
    _compose_action build
}

_build_no_cache_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    # build services
    _compose_action build --no-cache
}
_start_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    # start services
    _compose_action start
}

_rm_action ()
{
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    _compose_action rm -v
}

_setup_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    # start services
    _compose_action up --remove-orphans --no-start
}

_up_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    # stop services
    _compose_action stop

    # start services
    _compose_action up -d
}

_stop_action ()
{
    # update services if necessary
    _COMPOSE_SERVICE=${_COMPOSE_SERVICE:=`(_services_action)`}

    # stop services
    _compose_action stop
}

