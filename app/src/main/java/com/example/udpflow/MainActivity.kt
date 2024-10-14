package com.example.udpflow

import android.net.Uri
import android.os.Bundle
import android.os.CountDownTimer
import android.os.Environment
import android.text.method.ScrollingMovementMethod
import android.view.View
import android.view.View.OnClickListener
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.udpflow.databinding.ActivityMainBinding
import java.io.File
import java.io.FileOutputStream
import java.net.InetAddress
import java.net.NetworkInterface
import java.text.SimpleDateFormat
import java.util.Collections
import java.util.Date
import java.util.Locale


class MainActivity : AppCompatActivity(), OnClickListener {

    private lateinit var binding: ActivityMainBinding
    private val timer = object : CountDownTimer(Long.MAX_VALUE, 1000) {
        override fun onTick(millisUntilFinished: Long) {
            //  binding.editTextRecvBPS.text
            appendOutput(GetStat())
            onTick()
        }

        override fun onFinish() {}
    }

    private fun getIPAddress(useIPv4: Boolean): String? {
        try {
            val interfaces: List<NetworkInterface> =
                Collections.list(NetworkInterface.getNetworkInterfaces())
            for (intf in interfaces) {
                val addrs: List<InetAddress> = Collections.list(intf.inetAddresses)
                for (addr in addrs) {
                    if (!addr.isLoopbackAddress) {
                        val sAddr = addr.hostAddress
                        //boolean isIPv4 = InetAddressUtils.isIPv4Address(sAddr);
                        val isIPv4 = sAddr.indexOf(':') < 0
                        if (useIPv4) {
                            if (isIPv4) return sAddr
                        } else {
                            if (!isIPv4) {
                                val delim = sAddr.indexOf('%') // drop ip6 zone suffix
                                return if (delim < 0) sAddr.uppercase(Locale.getDefault()) else sAddr.substring(
                                    0,
                                    delim
                                ).uppercase(
                                    Locale.getDefault()
                                )
                            }
                        }
                    }
                }
            }
        } catch (ignored: Exception) {
        } // for now eat exceptions
        return ""
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.UdpServerSwitch.setOnClickListener(this)
        // Example of a call to a native method
        binding.textView.setMovementMethod(ScrollingMovementMethod())

        binding.textView.isSingleLine = false
        binding.textView.text = "Start UdpFlow. IP: " + getIPAddress(true)

        binding.buttonSave.setOnClickListener {
            onSaveStat()
        }

        SetOutput()

        ActivityCompat.requestPermissions(
            this@MainActivity, arrayOf<String>(
                android.Manifest.permission.WRITE_EXTERNAL_STORAGE,
                android.Manifest.permission.READ_EXTERNAL_STORAGE,
                android.Manifest.permission.MANAGE_EXTERNAL_STORAGE
            ),
            100
        )
    }

    fun appendOutput(string: String) {
        binding.textView.append('\n' + string)
    }

    fun onStartTimer() {
        timer.start();
    }

    fun onStopTimer() {
        timer.cancel();
    }

    override fun onClick(p0: View?) {

        if (UdpServerIsActive()) {
            UdpServerTurnOff()
        } else {
            UdpServerTurnOn()
        }
    }

    private fun onSaveStat() {
        var csvData = GetStatCSVDump()
        if (csvData.isNullOrEmpty())
            return

        var downloadsFolder =
            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
        val sdf = SimpleDateFormat("dd.MM.yyyy_hh_mm_ss")
        var fileName = "UdpFlow_Stat" + sdf.format(Date()) + ".csv"
        var file = File(downloadsFolder, fileName)
        file.createNewFile()

        val stream: FileOutputStream = FileOutputStream(file)
        try {
            stream.write(csvData.toByteArray())
            stream.close()
        } finally {
            stream.close()
        }
    }


    override fun onDestroy() {
        super.onDestroy()
        UdpServerTurnOff()
    }

    /**
     * A native method that is implemented by the 'udpflow' native library,
     * which is packaged with this application.
     */
    external fun SetOutput(): Void
    external fun UdpServerTurnOn(): Void
    external fun UdpServerTurnOff(): Void
    external fun UdpServerIsActive(): Boolean
    external fun GetStat(): String
    external fun onTick(): Void
    external fun GetStatCSVDump(): String


    companion object {
        // Used to load the 'udpflow' library on application startup.
        init {
            System.loadLibrary("udpflow")
        }
    }
}