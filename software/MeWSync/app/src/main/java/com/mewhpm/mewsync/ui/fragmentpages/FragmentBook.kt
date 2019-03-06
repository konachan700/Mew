package com.mewhpm.mewsync.ui.fragmentpages

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager

class FragmentBook private constructor() {
    companion object {
        private val fb: HashMap<Int, FragmentBook> = HashMap()
        fun getInstance(classId: Int) : FragmentBook {
            if (!fb.containsKey(classId)) fb[classId] = FragmentBook()
            return fb[classId]!!
        }
    }

    var hostViewResId: Int? = null
    var fragmentManager: FragmentManager? = null
    var groupTopFragmentRequest: (group: String) -> Fragment = {
        throw NotImplementedError("groupTopFragmentRequest not implemented now")
    }
    var currentFragment: Fragment? = null

    fun showTopInGroup(group: String) {
        currentFragment = groupTopFragmentRequest(group)
        val transaction = fragmentManager!!.beginTransaction()
        transaction.replace(hostViewResId!!, currentFragment!!)
        transaction.commit()
    }
}

fun androidx.appcompat.app.AppCompatActivity.getFragmentBook(hostViewResId: Int) : FragmentBook {
    val fb = FragmentBook.getInstance(this.hashCode())
    fb.fragmentManager = supportFragmentManager
    fb.hostViewResId = hostViewResId
    return fb
}

fun androidx.fragment.app.Fragment.getFragmentBook(hostViewResId: Int) : FragmentBook {
    val fb = FragmentBook.getInstance(this.hashCode())
    fb.fragmentManager = fragmentManager
    fb.hostViewResId = hostViewResId
    return fb
}