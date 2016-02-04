trafficserver-add_cache_lookup_status_header-plugin
===================================================

This is a Apache Traffic Server plugin to add the cache lookup status to the client response header.


## How to build

```
tsxs -o add_cache_lookup_status_header.so add_cache_lookup_status_header.c
```

The command `tsxs` is included in the `trafficserver-devel` rpm package.


## How to install

```
install add_cache_lookup_status_header.so /usr/lib64/trafficserver/plugins/ -m 0755 -o ats -g ats
```

## How to configure

Add the following line to `/etc/trafficserver/plugin.config`.

```
add_cache_lookup_status_header.so
```

You can specify the header name. The default value is `X-Cache`.

```
add_cache_lookup_status_header.so X-ATS-Cache
```

Run the following command to restart trafficserver.

```
systemctl restart trafficserver
```

## Cache lookup result header values

The value of cache lookup result header is one of the following:

* MISS
* HIT_FRESH
* HIT_STALE
* SKIPPED

This value is retrieved with [TSHttpTxnCacheLookupStatusGet](https://github.com/apache/trafficserver/blob/6.1.0/proxy/InkAPI.cc#L4753-L4781).
The cache hit ratio of multiple requests is calculated by (count-of-HIT_FRESH) / (count-of-HIT_FRESH + count-of-HIT_STALE + count-of-MISS).

## License
Apache License 2.0
