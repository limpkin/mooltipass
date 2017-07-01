package mooltipass.automatedTest.features;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;

import cucumber.api.java.en.Given;
import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Gmail;
import mooltipass.automatedTest.pageObjects.StackOverflow;

public class GmailSteps {
	Gmail gmail= new Gmail(WebDriverFactory.get());
	

	@When("I login Gmail with '(.*)'")
	public void login(String email){
		gmail.enterEmail(email);
		gmail.emailNextClick();
		String password =System.getenv().get("GMAILPASS");
		gmail.enterPassword(password);
		gmail.passwordNextClick();
	}
	@Then("I should be logged in Gmail")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",gmail.checkLogin());
	}
	
	@When("I logout Gmail")
	public void pressLogout(){
		gmail.logout();
	}
}
