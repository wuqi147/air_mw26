function timeup()
{
    self.postMessage("1");
}

self.addEventListener('message', function(e) {
    setTimeout(timeup, e.data);
}, false);
