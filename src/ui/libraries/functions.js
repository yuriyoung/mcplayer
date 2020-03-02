.pragma library

function formatTime(second) {
    return [parseInt(second / 60 / 60),
            parseInt(second / 60) % 60,
            second % 60].join(":").replace(/\b(\d)\b/g, "0$1");
}
