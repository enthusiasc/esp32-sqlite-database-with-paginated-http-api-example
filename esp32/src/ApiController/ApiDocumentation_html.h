const char API_DOCUMENTATION_HTML[] PROGMEM = R"=====(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <script type="module" src="https://unpkg.com/rapidoc/dist/rapidoc-min.js"></script>
</head>
<body>
  <rapi-doc
    spec-url="api.yaml"
    use-path-in-nav-bar="false"
    schema-description-expanded="true"
    default-schema-tab="example"
    show-header="false"
    text-color="#fff"
    bg-color="#222"
    info-description-headings-in-navbar = "true"
    layout="row"
    render-style="view"
    allow-try="false"
    allow-search="false"
    allow-authentication="false"
    allow-server-selection="false"
    allow-advanced-search="false"
    theme="dark"
  > </rapi-doc>
</body>
</html>
)=====";