package com.mewhpm.mewsync.fragments

import android.os.Bundle
import android.view.*
import com.google.gson.Gson
import com.mewhpm.mewsync.DeviceActivity
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.dao.PasswordsDao
import com.mewhpm.mewsync.dao.connectionSource
import com.mewhpm.mewsync.data.PassRecord
import com.mewhpm.mewsync.data.meta.PassRecordMetadata
import com.mewhpm.mewsync.data.enums.PassRecordType
import com.mewhpm.mewsync.utils.fixColorOfSearchBar
import com.mewhpm.mewsync.utils.hideKeyboard
import com.mikepenz.iconics.utils.IconicsMenuInflaterUtil
import kotlinx.android.synthetic.main.x02_fragment_passwords.view.*
import java.security.SecureRandom
import java.util.*

class PasswordsRootFragment : androidx.fragment.app.Fragment() {
    private var _view: View? = null
    private var _currentFolderId = 0L

    private val gson = Gson()
    private val srand = SecureRandom()

    private val onOkClick : (bundle: Bundle) -> Unit = { bundle ->
        val entity = createPassRecord(bundle)
        val dao = PasswordsDao.getInstance(this.requireContext().connectionSource)
        dao.createOrSave(entity)
    }

    private fun createPassRecord(bundle: Bundle) : PassRecord {
        val jsonPassRecord = bundle.getString(PasswordsAddElementFragment.JSON)
        return if (jsonPassRecord == null) PassRecord() else gson.fromJson(jsonPassRecord, PassRecord::class.java)
    }

    private fun refresh() {
        val dao = PasswordsDao.getInstance(this.requireContext().connectionSource)
        val list = dao.getAllChild(_currentFolderId)
        if (_view != null) {
            _view!!.listRVPasswords1.visibility = if (list.isEmpty() && _currentFolderId == 0L) View.GONE else View.VISIBLE
            _view!!.noItemsInList2.visibility = if (list.isEmpty() && _currentFolderId == 0L) View.VISIBLE else View.GONE
        }

        _view!!.listRVPasswords1.addItems(list, _currentFolderId == 0L)
    }

    override fun onResume() {
        super.onResume()
        refresh()
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        super.setHasOptionsMenu(true)
        _view = inflater.inflate(R.layout.x02_fragment_passwords, container, false)
        with (_view!!.listRVPasswords1) {
            create()
            onBackEvent = {
                val dao = PasswordsDao.getInstance(this@PasswordsRootFragment.requireContext().connectionSource)
                val parent = dao.getParent(_currentFolderId)
                _currentFolderId = parent?.id ?: 0
                refresh()
            }
            onItemClickEvent = { item ->
                when (item.recordType) {
                    PassRecordType.DIRECTORY -> {
                        _currentFolderId = item.id
                        refresh()
                    }
                    PassRecordType.PASSWORD -> { }
                }
            }
            onDeleteEvent = { record ->
                val dao = PasswordsDao.getInstance(this@PasswordsRootFragment.requireContext().connectionSource)
                when (record.recordType) {
                    PassRecordType.DIRECTORY -> dao.removeDir(record)
                    PassRecordType.PASSWORD -> dao.remove(record)
                }
                refresh()
            }
            onEditEvent = { record -> openEditor(record.recordType, record) }
        }
        return _view
    }

    private fun openEditor(type: PassRecordType, record: PassRecord? = null) {
        val editorFragment = PasswordsAddElementFragment()
        editorFragment.onOkClick = onOkClick

        val json = if (record != null) gson.toJson(record) else gson.toJson(PassRecord().also {
            it.recordType = type
            it.hwUID = srand.nextLong()
            it.deviceAddr = DeviceActivity.currentDeviceMac
            it.parentId = _currentFolderId
            it.timestamp = Date().time
            it.metadataJson = gson.toJson(PassRecordMetadata())
        })

        val bundle = Bundle()
        bundle.putString(PasswordsAddElementFragment.JSON, json)

        editorFragment.arguments = bundle
        activity!!.supportFragmentManager.beginTransaction()
            .replace(R.id.fragment_holder_dev_1, editorFragment)
            .addToBackStack("passEditor")
            .commit()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.menuCreateNewFolder1 -> { openEditor(PassRecordType.DIRECTORY) }
            R.id.menuCreateNewPassword1 -> { openEditor(PassRecordType.PASSWORD) }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        menu.clear()
        IconicsMenuInflaterUtil.inflate(inflater, this.requireContext(), R.menu.password_root_fragment_menu, menu)
        fixColorOfSearchBar(menu, R.id.passwords_fragment_search)
        val mItem: android.widget.SearchView = menu.findItem(R.id.passwords_fragment_search).actionView as android.widget.SearchView
        mItem.setOnQueryTextListener(object : android.widget.SearchView.OnQueryTextListener {
            override fun onQueryTextChange(newText: String): Boolean = false
            override fun onQueryTextSubmit(query: String): Boolean {

                mItem.clearFocus()
                _view?.hideKeyboard()
                return true
            }
        })
    }
}