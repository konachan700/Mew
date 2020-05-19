package com.mewhpm.mewsync.fragments

import android.os.Bundle
import android.view.*
import com.google.gson.Gson
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.data.PassRecord
import com.mewhpm.mewsync.data.meta.PassRecordMetadata
import com.mewhpm.mewsync.data.enums.PassRecordType
import com.mewhpm.mewsync.utils.hideKeyboard
import com.mikepenz.iconics.utils.IconicsMenuInflaterUtil
import kotlinx.android.synthetic.main.x02_fragment_add_directory.view.*
import kotlinx.android.synthetic.main.x02_fragment_add_password.view.*
import org.jetbrains.anko.support.v4.toast

class PasswordsAddElementFragment : androidx.fragment.app.Fragment() {
    companion object {
        const val MAX_PASSWORD_LEN_INPUT = 80
        const val PRIMARY_TEXT_MAX_LEN = 40
        const val JSON = "object_json"
    }

    private var _view: View? = null
    private val gson = Gson()
    private var currentRecord : PassRecord = PassRecord()
    private var currentMeta : PassRecordMetadata =
        PassRecordMetadata()

    var onOkClick : (bundle: Bundle) -> Unit = {}

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.menuCreateNewElementCancel -> {
                _view!!.hideKeyboard()
                activity!!.supportFragmentManager.popBackStack()
            }
            R.id.menuCreateNewElementOk -> {
                val bundle = Bundle()
                when (currentRecord.recordType) {
                    PassRecordType.DIRECTORY -> {
                        val dirName = _view!!.editTextDirectoryName.text.toString()
                        if (dirName.isBlank()) {
                            toast("Directory name can't be blank").show()
                            return super.onOptionsItemSelected(item)
                        }
                        if (dirName.length > PRIMARY_TEXT_MAX_LEN) {
                            toast("Directory name must be a 1-$PRIMARY_TEXT_MAX_LEN chars").show()
                            return super.onOptionsItemSelected(item)
                        }
                        currentRecord.title = dirName
                        //currentMeta.text = _view!!.editTextDirectoryDescription.text.toString()
                    }
                    PassRecordType.PASSWORD -> {
                        val desc = _view!!.editTextPassDescription.text.toString()
                        if (desc.isBlank()) {
                            toast(getString(R.string.password_not_be_blank)).show()
                            return super.onOptionsItemSelected(item)
                        }
                        if (desc.length > PRIMARY_TEXT_MAX_LEN) {
                            toast("Password description must be a 1-$PRIMARY_TEXT_MAX_LEN chars").show()
                            return super.onOptionsItemSelected(item)
                        }
                        currentRecord.title = desc

                        with (_view!!) {
                            currentMeta.url = editTextPassURL.text.toString()
                            currentMeta.login = editTextPassLogin.text.toString()
                            currentMeta.containDigits = checkBoxDigits.isChecked
                            currentMeta.containSymbols = checkBoxSymbols.isChecked
                            currentMeta.containUppercaseAlphabet = checkBoxAlphaUpper.isChecked
                            currentMeta.containLowercaseAlphabet = checkBoxAlphaLower.isChecked

                            try {
                                currentMeta.minLen = Integer.parseInt(editTextMinLen.text.toString())
                                currentMeta.maxLen = Integer.parseInt(editTextMaxLen.text.toString())
                                if (currentMeta.minLen <= 2) {
                                    toast("Password is too small, minimum 3 chars allowed").show()
                                    return super.onOptionsItemSelected(item)
                                }

                                if (currentMeta.maxLen > MAX_PASSWORD_LEN_INPUT) {
                                    toast("Password is too big, maximum $MAX_PASSWORD_LEN_INPUT chars allowed").show()
                                    return super.onOptionsItemSelected(item)
                                }
                            } catch (e : NumberFormatException) {
                                toast("Invalid max/min value entered").show()
                                return super.onOptionsItemSelected(item)
                            }
                        }
                    }
                }
                val metaJson = gson.toJson(currentMeta)
                currentRecord.metadataJson = metaJson

                val json = gson.toJson(currentRecord)
                bundle.putString(JSON, json)

                onOkClick.invoke(bundle)

                _view!!.hideKeyboard()
                activity!!.supportFragmentManager.popBackStack()
            }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        menu.clear()
        IconicsMenuInflaterUtil.inflate(inflater, this.requireContext(), R.menu.password_add_element_fragment_menu, menu)
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        super.setHasOptionsMenu(true)

        val json = arguments!!.getString(JSON, null)
        if (json != null && json.isNotBlank()) {
            currentRecord = gson.fromJson(json, PassRecord::class.java) ?: PassRecord()
            currentMeta = gson.fromJson<PassRecordMetadata>(currentRecord.metadataJson, PassRecordMetadata::class.java) ?: PassRecordMetadata()

            when (currentRecord.recordType) {
                PassRecordType.DIRECTORY -> {
                    _view = inflater.inflate(R.layout.x02_fragment_add_directory, container, false)
                    with (_view!!) {
                        editTextDirectoryName.setText(currentRecord.title)
                        //editTextDirectoryDescription.setText(currentMeta.text)
                    }
                }
                PassRecordType.PASSWORD -> {
                    _view = inflater.inflate(R.layout.x02_fragment_add_password, container, false)

                    with (_view!!) {
                        editTextPassURL.setText(currentMeta.url)
                        editTextPassLogin.setText(currentMeta.login)
                        editTextPassDescription.setText(currentRecord.title)

                        checkBoxAlphaLower.isChecked = currentMeta.containLowercaseAlphabet
                        checkBoxAlphaUpper.isChecked = currentMeta.containUppercaseAlphabet
                        checkBoxSymbols.isChecked = currentMeta.containSymbols
                        checkBoxDigits.isChecked = currentMeta.containDigits

                        editTextMinLen.setText(currentMeta.minLen.toString(10))
                        editTextMaxLen.setText(currentMeta.maxLen.toString(10))
                    }
                }
            }
        }
        return _view
    }
}