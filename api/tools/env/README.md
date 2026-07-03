## PyServer development environment

> [!Warning]
> This is a testing environment, do not use in production.

This folder contains the development environment for the GuardSarm 4.x versions. It includes the following components:

- **guardsarm-master**: Master node.
- **guardsarm-worker1**: Worker node #1.
- **guardsarm-worker2**: Worker node #2.
- **nginx-lb**: NGINX load balancer to distribute agent connections among nodes.
- **guardsarm-agent**: Agent.
- **guardsarm-indexer**: Indexer

### Working with docker environment
The following commands runs a cluster:
1. Run `docker compose build`
2. Run `docker compose up`

If a single docker is needed, it is possible to run:
1. Move to the dockefile location for instance:
 `cd guardsarm-manager`
2. Run `docker build -t guardsarm-manager --target server ./guardsarm-manager`
3. Define .env with the necessary environment variables
4. Run docker:
```
docker run -d \
--name guardsarm-master \
--hostname guardsarm-master \
-p 55000:55000 \
-v ${GUARDSARM_LOCAL_PATH}/framework/scripts:/var/guardsarm-manager/framework/scripts \
-v ${GUARDSARM_LOCAL_PATH}/api/scripts:/var/guardsarm-manager/api/scripts \
-v ${GUARDSARM_LOCAL_PATH}/framework/guardsarm:/var/guardsarm-manager/framework/python/lib/python${GUARDSARM_PYTHON_VERSION}/site-packages/guardsarm \
-v ${GUARDSARM_LOCAL_PATH}/api/api:/var/guardsarm-manager/framework/python/lib/python${GUARDSARM_PYTHON_VERSION}/site-packages/api \
guardsarm-manager \
/scripts/entrypoint.sh guardsarm-master master-node master
```
If we need more agents we can use:
`docker compose up --scale guardsarm-agent=<number_of_agents>`

### Troubleshooting
- Use option **-no-cache** when you have building issues.

`docker compose build --no-cache`

- To completely clean the environment (including volumes), run:
  ```bash
  docker compose down -v
  ```
