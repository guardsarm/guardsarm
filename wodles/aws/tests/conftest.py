# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

import pytest
import sqlite3

@pytest.fixture()
def custom_database():
    """Create a custom database in memory without cache."""
    memory_db = sqlite3.connect(':memory:')
    yield memory_db

    # List tables
    table_list = memory_db.execute(
        "SELECT name FROM sqlite_master WHERE type ='table' AND name NOT LIKE 'sqlite_%';").fetchall()

    # Drop tables
    for table in [table[0] for table in table_list]:
        memory_db.execute(f"DROP TABLE {table};")
    memory_db.close()
