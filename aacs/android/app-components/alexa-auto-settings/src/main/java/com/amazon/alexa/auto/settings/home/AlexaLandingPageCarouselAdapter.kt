package com.amazon.alexa.auto.settings.home

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.amazon.alexa.auto.settings.databinding.LandingPageCarouselContentBinding

class AlexaLandingPageCarouselAdapter(val carouselList: ArrayList<UpperContent>):
    RecyclerView.Adapter<AlexaLandingPageCarouselAdapter.ViewHolder>() {

    inner class ViewHolder(val binding: LandingPageCarouselContentBinding): RecyclerView.ViewHolder(binding.root)

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        return ViewHolder(LandingPageCarouselContentBinding.inflate(LayoutInflater.from(parent.context), parent, false))
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val currUpperContent: UpperContent = carouselList[position]
        with(holder) {
            binding.upperContentLogo.setImageDrawable(currUpperContent.drawable)
            binding.upperContentTitle.text = currUpperContent.primaryText
            binding.upperContentBody.text = currUpperContent.secondaryText
            binding.upperContentButton.text = currUpperContent.btnText
            if (binding.upperContentButton.text.isEmpty()) {
                binding.upperContentButton.visibility = View.INVISIBLE
                binding.upperContentButton.isEnabled = false
                binding.upperContentButton.isClickable = false
            } else {
                binding.upperContentButton.visibility = View.VISIBLE
                binding.upperContentButton.isEnabled = true
                binding.upperContentButton.isClickable = true
            }
            binding.upperContentButton.setOnClickListener(currUpperContent.btnCallback)
        }
    }

    override fun getItemCount(): Int {
        return carouselList.size
    }

}