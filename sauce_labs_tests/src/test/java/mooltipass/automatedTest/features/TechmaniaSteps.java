package mooltipass.automatedTest.features;

import cucumber.api.java.en.Given;
import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Techmania;;

public class TechmaniaSteps {

	Techmania techmania= new Techmania(WebDriverFactory.get());
	
	
	@When("I log in Techmania with '(.*)'")
	public void login(String username){
		techmania.clickloginLink();
		techmania.enterUsername(username);
		String password = System.getenv().get("TECHPASS");
		techmania.enterPassword(password);
		techmania.submit();
	}

	
	@Then("I should be logged in Techmania")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",techmania.checkLogin());
	}
	
	@When("I go to Techmania login page")
	public void pressLogin(){
		techmania.clickloginLink();
		Assert.assertFalse("Expected to be at login page", techmania.checkLogin());
		
	}
	@When("I logout Techmania")
	public void pressLogout(){
		techmania.logout();
	}
}
