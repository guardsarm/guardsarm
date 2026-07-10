# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from typing import Union

from guardsarm.core import common
from guardsarm.core.cluster import local_client
from guardsarm.core.cluster.cluster import get_node
from guardsarm.core.cluster.control import get_health, get_nodes
from guardsarm.core.cluster.utils import get_cluster_status, read_config
from guardsarm.core.exception import GuardSarmError, GuardSarmResourceNotFound
from guardsarm.core.results import AffectedItemsGuardSarmResult, GuardSarmResult
from guardsarm.rbac.decorators import expose_resources, async_list_handler

node_id = get_node().get('node')


@expose_resources(actions=['cluster:read'], resources=[f'node:id:{node_id}'])
def read_config_wrapper() -> AffectedItemsGuardSarmResult:
    """Wrapper for read_config.

    Returns
    -------
    AffectedItemsGuardSarmResult
        Affected items.
    """
    result = AffectedItemsGuardSarmResult(all_msg='All selected information was returned',
                                      none_msg='No information was returned'
                                      )
    try:
        result.affected_items.append(read_config())
    except GuardSarmError as e:
        result.add_failed_item(id_=node_id, error=e)
    result.total_affected_items = len(result.affected_items)

    return result


@expose_resources(actions=['cluster:read'], resources=[f'node:id:{node_id}'])
def get_node_wrapper() -> AffectedItemsGuardSarmResult:
    """Wrapper for get_node.

    Returns
    -------
    AffectedItemsGuardSarmResult
        Affected items.
    """
    result = AffectedItemsGuardSarmResult(all_msg='All selected information was returned',
                                      none_msg='No information was returned'
                                      )
    try:
        result.affected_items.append(get_node())
    except GuardSarmError as e:
        result.add_failed_item(id_=node_id, error=e)
    result.total_affected_items = len(result.affected_items)

    return result


@expose_resources(actions=['cluster:status'], resources=['*:*:*'], post_proc_func=None)
def get_status_json() -> GuardSarmResult:
    """Return the cluster status.

    Returns
    -------
    GuardSarmResult
        GuardSarmResult object with the cluster status.
    """
    return GuardSarmResult({'data': get_cluster_status()})


@expose_resources(actions=['cluster:read'], resources=['node:id:{filter_node}'], post_proc_func=async_list_handler)
async def get_health_nodes(lc: local_client.LocalClient,
                           filter_node: Union[str, list] = None) -> AffectedItemsGuardSarmResult:
    """Wrapper for get_health.

    Parameters
    ----------
    lc : LocalClient object
        LocalClient with which to send the 'get_nodes' request.
    filter_node : str or list
        Node to return.

    Returns
    -------
    AffectedItemsGuardSarmResult
        Affected items.
    """
    result = AffectedItemsGuardSarmResult(all_msg='All selected nodes healthcheck information was returned',
                                      some_msg='Some nodes healthcheck information was not returned',
                                      none_msg='No healthcheck information was returned'
                                      )

    data = await get_health(lc, filter_node=filter_node)
    for v in data['nodes'].values():
        result.affected_items.append(v)

    result.affected_items = sorted(result.affected_items, key=lambda i: i['info']['name'])
    result.total_affected_items = len(result.affected_items)

    return result


@expose_resources(actions=['cluster:read'], resources=['node:id:{filter_node}'], post_proc_func=async_list_handler)
async def get_nodes_info(lc: local_client.LocalClient, filter_node: Union[str, list] = None,
                         **kwargs: dict) -> AffectedItemsGuardSarmResult:
    """Wrapper for get_nodes.

    Parameters
    ----------
    lc : LocalClient object
        LocalClient with which to send the 'get_nodes' request.
    filter_node : str or list
        Node to return.

    Returns
    -------
    AffectedItemsGuardSarmResult
        Affected items.
    """
    result = AffectedItemsGuardSarmResult(all_msg='All selected nodes information was returned',
                                      some_msg='Some nodes information was not returned',
                                      none_msg='No information was returned'
                                      )

    nodes = set(filter_node).intersection(set(common.cluster_nodes.get()))
    non_existent_nodes = set(filter_node) - nodes
    data = await get_nodes(lc, filter_node=list(nodes), **kwargs)
    for item in data['items']:
        result.affected_items.append(item)

    for node in non_existent_nodes:
        result.add_failed_item(id_=node, error=GuardSarmResourceNotFound(1730))
    result.total_affected_items = data['totalItems']

    return result
