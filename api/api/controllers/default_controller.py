# # Copyright (C) 2015, Wazuh Inc.
# # Created by Wazuh, Inc. <info@wazuh.com>.
# # This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

import logging
import socket

from connexion.lifecycle import ConnexionResponse

from api.controllers.util import json_response
from api.models.basic_info_model import BasicInfo
from guardsarm.core.common import DATE_FORMAT
from guardsarm.core.results import GuardSarmResult
from guardsarm.core.security import load_spec
from guardsarm.core.utils import get_utc_now

logger = logging.getLogger('guardsarm-api')


async def default_info(pretty: bool = False) -> ConnexionResponse:
    """Return basic information about the GuardSarm API.

    Parameters
    ----------
    pretty: bool
        Show results in human-readable format.

    Returns
    -------
    ConnexionResponse
        API response.
    """
    info_data = load_spec()
    data = {
        'title': info_data['info']['title'],
        'api_version': info_data['info']['version'],
        'revision': info_data['info']['x-revision'],
        'license_name': info_data['info']['license']['name'],
        'license_url': info_data['info']['license']['url'],
        'hostname': socket.gethostname(),
        'timestamp': get_utc_now().strftime(DATE_FORMAT)
    }
    data = GuardSarmResult({'data': BasicInfo.from_dict(data)})

    return json_response(data, pretty=pretty)
