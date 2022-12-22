/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.templateruntime.bodytemplate

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import coil.load
import com.amazon.aacsconstants.TemplateRuntimeConstants
import com.amazon.aacsipc.AACSSender
import com.amazon.alexa.auto.aacs.common.*
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apis.session.SessionViewController
import com.amazon.alexa.auto.templateruntime.R
import com.amazon.alexa.auto.templateruntime.dependencies.TemplateDirectiveHandler
import com.amazon.alexa.auto.templateruntime.receiver.AlexaVoiceoverCompletedMessage
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import java.lang.ref.WeakReference

class BodyTemplateDirectiveHandler(context: Context) :
    TemplateDirectiveHandler {
    private val mMessageSender: AACSMessageSender
    private val mAACSSender: AACSSender
    private val mContext: WeakReference<Context>

    companion object {
        private val TAG = BodyTemplateDirectiveHandler::class.java.simpleName
        private const val MAX_DISPLAY_CHARS = 80
        private val mHandler = Handler()
    }

    init {
        mContext = WeakReference(context)
        mAACSSender = AACSSender()
        mMessageSender = AACSMessageSender(mContext, mAACSSender)
    }

    override fun renderTemplate(message: AACSMessage) {
        TemplateDirectiveHandler.clearTemplate(mContext)
        val app = AlexaApp.from(mContext.get()!!)
        app.rootComponent.getComponent(SessionViewController::class.java)
            .ifPresent { sessionViewController: SessionViewController ->
                sessionViewController.templateRuntimeViewContainer.ifPresent { viewGroup: ViewGroup ->
                    TemplateRuntimeMessages.parseBodyTemplate(message.payload)
                        .ifPresent { bodyTemplate: BodyTemplate ->
                            try {
                                renderBodyTemplateView(viewGroup, bodyTemplate, mContext)
                                sessionViewController.setTemplateDisplayed()
                                EventBus.getDefault().register(this)
                            } catch (e: Exception) {
                                Log.e(
                                    TAG,
                                    "Issue inflating template: $e"
                                )
                            }
                        }
                }
            }
    }

    private fun renderBodyTemplateView(
        viewGroup: ViewGroup, bodyTemplate: BodyTemplate, mContext: WeakReference<Context>
    ) {
        val layoutInflater =
            mContext.get()!!.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        viewGroup.addOnAttachStateChangeListener(onAttachStateChangeListener)
        val inflatedView = layoutInflater.inflate(R.layout.body_template, null)
        inflatedView.id = R.id.body_template_view
        viewGroup.addView(
            inflatedView,
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.MATCH_PARENT
        )
        removeClearTemplateCallback()
        val mainTitleText = inflatedView.findViewById<TextView>(R.id.body_template_card_mainTitle)
        mainTitleText.text = bodyTemplate.title.mainTitle
        val subTitleText = inflatedView.findViewById<TextView>(R.id.body_template_card_subTitle)
        subTitleText.text = bodyTemplate.title.subTitle
        val textField: TextView
        if (bodyTemplate.image != null) {
            val imageUrl: String = try {
                bodyTemplate.image!!
                    .sources
                    .stream()
                    .filter { (size): Source ->
                        size == TemplateRuntimeConstants.IMAGE_SIZE_LARGE
                    }
                    .findFirst()
                    .get()
                    .url
            } catch (e: NoSuchElementException) {
                bodyTemplate.image!!.sources[0].url
            }
            val imageView = inflatedView.findViewById<ImageView>(R.id.body_template_card_image)
            imageView.load(imageUrl)
            inflatedView.findViewById<View>(R.id.body_template_card_text_field).visibility =
                View.INVISIBLE
            textField = inflatedView.findViewById(R.id.body_template_card_text_field_image)
        } else {
            inflatedView.findViewById<View>(R.id.body_template_card_image).visibility =
                View.INVISIBLE
            inflatedView.findViewById<View>(R.id.body_template_card_text_field_image).visibility =
                View.INVISIBLE
            textField = inflatedView.findViewById(R.id.body_template_card_text_field)
        }

        if (bodyTemplate.skillIcon != null) {
            val iconUrl: String = try {
                bodyTemplate.skillIcon!!
                    .sources
                    .stream()
                    .filter { (size): Source ->
                        size == TemplateRuntimeConstants.IMAGE_SIZE_SMALL
                    }
                    .findFirst()
                    .get()
                    .url
            } catch (e: NoSuchElementException) {
                bodyTemplate.skillIcon!!.sources[0].url
            }
            val skillIconView = inflatedView.findViewById<ImageView>(R.id.body_template_skill_icon)
            skillIconView.load(iconUrl)
        }

        textField.text = getDisplayText(mContext, bodyTemplate.textField)
        val closeButton = inflatedView.findViewById<ImageView>(R.id.close_button)
        closeButton.setOnClickListener {
            TemplateDirectiveHandler.clearTemplateAndEndVoiceActivity(mMessageSender, mContext)
        }
    }

    private fun getDisplayText(mContext: WeakReference<Context>, textFieldString: String): String {
        var displayTextString = textFieldString
        return if (displayTextString.length <= MAX_DISPLAY_CHARS) {
            displayTextString
        } else {
            displayTextString = displayTextString.substring(0, MAX_DISPLAY_CHARS)
            if (displayTextString[MAX_DISPLAY_CHARS - 1] != ' ') {
                displayTextString = displayTextString.substring(0, displayTextString.lastIndexOf(" "))
            }
            displayTextString + mContext.get()!!.resources.getString(R.string.ellipsis)
        }
    }

    // clear existing callbacks and event subscribers, if any
    private val onAttachStateChangeListener: View.OnAttachStateChangeListener
        get() = object : View.OnAttachStateChangeListener {
            override fun onViewAttachedToWindow(v: View) {}
            override fun onViewDetachedFromWindow(v: View) {
                // clear existing callbacks and event subscribers, if any
                removeClearTemplateCallback()
                EventBus.getDefault().unregister(this@BodyTemplateDirectiveHandler)
            }
        }

    fun removeClearTemplateCallback() {
        mHandler.removeCallbacksAndMessages(null)
    }

    /**
     * Subscribe to event published by
     * [com.amazon.alexa.auto.templateruntime.receiver.AlexaStateChangeReceiver] when Alexa
     * has completed its voice response to the user's weather utterance
     *
     * @param message - Message published by event publisher
     */
    @Subscribe
    fun OnReceive(message: AlexaVoiceoverCompletedMessage?) {
        startClearTemplateDelay()
    }

    private fun startClearTemplateDelay() {
        removeClearTemplateCallback()
        mHandler.postDelayed(
            clearBodyTemplateRunnable,
            TemplateDirectiveHandler.CLEAR_TEMPLATE_DELAY_MS.toLong()
        )
    }

    private var clearBodyTemplateRunnable = Runnable {
        AlexaApp.from(mContext.get()!!)
            .rootComponent
            .getComponent(SessionViewController::class.java)
            .ifPresent { sessionViewController: SessionViewController ->
                sessionViewController.templateRuntimeViewContainer.ifPresent { viewGroup: ViewGroup ->
                    if (viewGroup.findViewById<View?>(R.id.body_template_view) != null) {
                        sessionViewController.clearTemplate()
                    }
                }
            }
    }
}