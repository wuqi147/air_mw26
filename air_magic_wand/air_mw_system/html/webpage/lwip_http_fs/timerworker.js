const THREAD_ACTIVE_COUNTER_INIT_VALUE = 10;
const HTML_SEND_COMPLETE = 4; // XMLHttpRequest.DONE
const HTML_STATUS_OK = 200; // HTTP OK
var timerInterval = 0;
var curXml = null;
var IS_READY = 1;
var doUpgrade = false;
var mainThreadActiveCounter = THREAD_ACTIVE_COUNTER_INIT_VALUE;
let intervalId = null;

function xmlSend()
{
    if (curXml == null)
    {
         curXml = new XMLHttpRequest();
    }
    if (curXml != null && IS_READY === 1 && doUpgrade == false)
    {
        curXml.onreadystatechange = postXMLRsp;
        curXml.open("GET", "/topLed.xml", true);
        curXml.send(null);
        IS_READY = 0;
    }
    if (mainThreadActiveCounter !== 0)
    {
        mainThreadActiveCounter--;
    }
}
function postXMLRsp()
{
    if (curXml.readyState === HTML_SEND_COMPLETE)
    {
        if ((curXml.status === HTML_STATUS_OK) && (mainThreadActiveCounter !== 0))
        {
            var xmlRspData = {
                readyState: curXml.readyState,
                status: curXml.status,
                responseText: curXml.responseText
            };

            self.postMessage(xmlRspData);
        }

        IS_READY = 1;
    }
}
self.addEventListener('message', function(event)
{
    doUpgrade = event.data.doUpgrade;
    if (event.data.timerInterval !== timerInterval)
    {
        timerInterval = event.data.timerInterval;
        if (intervalId !== null)
        {
            clearInterval(intervalId);
            intervalId = null;
        }

        intervalId = setInterval(xmlSend, timerInterval);
    }
    mainThreadActiveCounter = THREAD_ACTIVE_COUNTER_INIT_VALUE;
}, false);
