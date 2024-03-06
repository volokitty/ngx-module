# ngx-decline-module

This handler module responds to requests with the given status code, in case the query params contain all the fields specified in the `json` configuration.

## Build

Tested on macOS.

Add the module from nginx source directory:

```
./auto/configure --add-module=/path/to/ngx-module
```

Then build it:

```
make
make install
```

## Configuration

This module works only in `location` configuration (decline directive). So here is a simple example of `nginx.conf`:

```
server {
  listen 8080;
  server_name example.com;

  location / {
    root /var/www/html;
    index index.html;
  }

  location /decline {
    decline "/path/to/config.json";
  }
}
```

Example of `config.json`:

```
{
  "get": {
    "code": 403,
    "content": ["username", "password"]
  },
  "post": {
    "code": 401,
    "content": ["city"]
  }
}
```

So it will respond with **403** code to **GET** `http://localhost:8080/decline?username="ulad"&password="qwerty"` and with **401** code to **POST** `http://localhost:8080/decline?city="moscow"`.

Everything else is **204** No Content.
