# coding: utf-8

# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from datetime import date, datetime  # noqa: F401
from typing import List, Dict  # noqa: F401

from api.models.base_model_ import Body


class GroupAddedModel(Body):

    def __init__(self, group_id: str = None):
        """GroupAddedModel body model.

        Parameters
        ----------
        group_id : str
            Group name.
        """
        self.swagger_types = {
            'group_id': str,
        }

        self.attribute_map = {
            'group_id': 'group_id',
        }

        self._group_id = group_id

    @property
    def group_id(self) -> str:
        """Group name getter.
        Returns
        -------
        group_id : str
            Group name.
        """
        return self._group_id

    @group_id.setter
    def group_id(self, group_id):
        """Group name setter.
        Parameters
        ----------
        group_id : str
            Group name.
        """
        self._group_id = group_id
