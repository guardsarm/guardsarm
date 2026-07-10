# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from functools import wraps


def dapi_allower(is_async: bool = False):
    """Decorator to allow execution of a function through the distributed GuardSarm API.

    Parameters
    ----------
    is_async : bool
        Whether the decorated function is asynchronous.
    """

    def decorator(func: callable):

        if is_async:

            @wraps(func)
            async def wrapper(*args, **kwargs):
                return await func(*args, **kwargs)

        else:

            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)

        wrapper.__guardsarm_exposed__ = True
        return wrapper

    return decorator
