package com.amazon.alexa.auto.settings

import android.util.Log
import androidx.recyclerview.widget.RecyclerView
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.selection.ItemDetailsLookup
import androidx.recyclerview.selection.SelectionTracker
import com.amazon.alexa.auto.settings.databinding.FragmentSettingsLanguageItemBinding
import com.amazon.alexa.auto.settings.home.AlexaLandingPageFragment

class SettingsAlexaLanguageAdapter(
    val localesList: List<Locale>
) : RecyclerView.Adapter<SettingsAlexaLanguageAdapter.ViewHolder>() {

    var tracker: SelectionTracker<String>? = null
    companion object {
        private val TAG: String = AlexaLandingPageFragment::class.java.canonicalName as String
    }


    init {
        setHasStableIds(true)
    }

    inner class ViewHolder(val binding: FragmentSettingsLanguageItemBinding) : RecyclerView.ViewHolder(binding.root) {
        fun getItemDetails(): ItemDetailsLookup.ItemDetails<String> = object :
            ItemDetailsLookup.ItemDetails<String>() {
            override fun getPosition(): Int = bindingAdapterPosition
            override fun getSelectionKey(): String = localesList[bindingAdapterPosition].localeID
            override fun inSelectionHotspot(e: MotionEvent): Boolean = true
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        return ViewHolder(FragmentSettingsLanguageItemBinding.inflate(LayoutInflater.from(parent.context), parent, false))
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val item = localesList[position]
        with (holder) {
            binding.title.text = item.language
            binding.summary.text =item.country

            tracker?.let {
                if (it.isSelected(item.localeID)) {
                    Log.d(TAG, "SELECTED LOCALE: ${item.localeID}")
                    it.select(item.localeID)
                    binding.root.isActivated = true
                    binding.widgetFrame.visibility = View.VISIBLE
                    binding.topSelectionBar.visibility = View.VISIBLE
                    binding.bottomSelectionBar.visibility = View.VISIBLE
                } else {
                    it.deselect(item.localeID)
                    binding.root.isActivated = false
                    binding.widgetFrame.visibility = View.INVISIBLE
                    binding.topSelectionBar.visibility = View.INVISIBLE
                    binding.bottomSelectionBar.visibility = View.INVISIBLE
                }
            }

        }
    }

    override fun getItemCount(): Int = localesList.size
    override fun getItemId(position: Int): Long = position.toLong()

}