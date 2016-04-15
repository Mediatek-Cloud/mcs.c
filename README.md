# mcs.c

## API

* `void mcs_upload_datapoint(char *);`
  - Upload data points to MCS.
* `void mcs_tcp_init(void (*mcs_tcp_callback)(char *));`
  - Listen the TCP command from MCS.
* `smart_config_if_enabled()`
  - smart connection with MCS app.

## Usage

* copy mcs.c, mcs.h, smart_connection.c, smart_connection.h to your `/project/mt7687_hdk/{your project name}/`

* Following, we assume `/project/mt7687_hdk/{your project name}/` is the `{project_root}` path.

* If you want to use the smart connection feature, edit the `{project_root}/GCC/feature.mk`:

```
  # add this line:
  MTK_SMARTCONNECT_HDK = y
  # Please reference mcs.c/reference/feature.mk, line24

```
* Edit `{project_root}/main.c`:

```
  # add this line:
  # Please reference mcs.c/reference/main.c, line24
```

* Edit `{project_root/GCC/Makefile}:

```
  # add this line:

  # Please reference mcs.c/reference/Makefile, line24
```   
