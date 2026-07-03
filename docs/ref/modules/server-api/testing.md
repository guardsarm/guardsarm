# Testing

The Server API and Framework use `pytest` as the test runner. Tests are organized alongside each module.

---

## Test Locations

| Location | Scope |
|----------|-------|
| `framework/guardsarm/tests/` | Interface layer unit tests |
| `framework/guardsarm/core/tests/` | Core logic unit tests |
| `api/api/test/` | API layer unit tests |
| `api/api/controllers/test/` | Controller tests |
| `framework/guardsarm/rbac/tests/` | RBAC unit tests |
| `framework/guardsarm/core/indexer/tests/` | Indexer integration tests |

---

## Running Tests

```bash
export GUARDSARM_REPO=<your_path>
PYTHONPATH=$GUARDSARM_REPO/framework:$GUARDSARM_REPO/api pytest framework --disable-warnings
```

### Running specific test modules

```bash
# Interface layer tests
PYTHONPATH=$GUARDSARM_REPO/framework:$GUARDSARM_REPO/api pytest framework/guardsarm/tests/ --disable-warnings

# Core logic tests
PYTHONPATH=$GUARDSARM_REPO/framework:$GUARDSARM_REPO/api pytest framework/guardsarm/core/tests/ --disable-warnings

# API layer tests
PYTHONPATH=$GUARDSARM_REPO/framework:$GUARDSARM_REPO/api pytest api/api/test/ --disable-warnings
```

---

## Configuration

The project uses `pytest.ini` files for test configuration. These are located at:
- `framework/pytest.ini`
- `api/api/pytest.ini`
